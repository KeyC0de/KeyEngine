#include <sstream>
#include <mutex>
#include "dxgi1_4.h"
#include "dxgi1_5.h"
#include "graphics.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "render_target.h"
#include "utils.h"
#include "os_utils.h"
#include "settings_manager.h"
#include "console.h"
#include "graphics_mode.h"
#include "rectangle.h"
#include <ittnotify.h>	// Intel Instrumentation & Tracing Technology
#include "vtune_itt_domain.h"

#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "DirectXTK.lib" )

#if defined _DEBUG && !defined NDEBUG
#	define DXGI_GET_QUEUE_INFO_GFX \
	{\
		KeyConsole& console = KeyConsole::getInstance();\
		const auto& messages = getInfoQueue().getInfoMessages();\
		if ( !messages.empty() )\
		{\
			for ( const auto& msg : messages )\
			{\
				console.log( msg + "\n" );\
			}\
			__debugbreak();\
		}\
		getInfoQueue().markQueueIndex();\
	}
#else
#	define DXGI_GET_QUEUE_INFO_GFX (void)0;
#endif


namespace mwrl = Microsoft::WRL;
namespace dx = DirectX;

// Create ITT string handles
__itt_string_handle* pStrIttDrawIndexed = __itt_string_handle_create( L"DrawIndexed" );
__itt_string_handle* pStrIttDrawIndexedInstanced = __itt_string_handle_create( L"DrawIndexedInstanced" );
__itt_string_handle* pStrIttBeginRendering = __itt_string_handle_create( L"BeginRendering" );
__itt_string_handle* pStrIttFpsTimerRendering = __itt_string_handle_create( L"FpsTimerRendering" );
__itt_string_handle* pStrIttEndRendering = __itt_string_handle_create( L"EndRendering" );

#if defined _DEBUG && !defined NDEBUG
static std::once_flag g_startUpFlag;
#endif

auto& g_settings = SettingsManager::getInstance();


Graphics::Adapter::Adapter( IDXGIAdapter* pAdapter )
{
	m_pAdapter = pAdapter;
	pAdapter->GetDesc( &m_desc );
}

const DXGI_ADAPTER_DESC* Graphics::Adapter::getDesc() const noexcept
{
	return &m_desc;
}

IDXGIAdapter* Graphics::Adapter::getAdapter() const noexcept
{
	return m_pAdapter.Get();
}

#if defined _FLIP_PRESENT
void Graphics::makeWindowAssociationWithFactory( HWND hWnd,
	UINT flags )
{
	m_pSwapChain->GetParent( __uuidof( IDXGIFactory ),
		&m_pFactory );
	m_pFactory->MakeWindowAssociation( hWnd,
		flags );
}

bool Graphics::checkTearingSupport()
{
	bool bAllowTearing = false;
	// Rather than create the 1.5 factory interface directly, we create the 1.4
	// interface and query for the 1.5 interface. This will enable the graphics
	// debugging tools which might not support the 1.5 factory interface.
	mwrl::ComPtr<IDXGIFactory4> factory4;
	HRESULT hres = CreateDXGIFactory1( IID_PPV_ARGS( &factory4 ) );
	ASSERT_HRES_IF_FAILED;

	mwrl::ComPtr<IDXGIFactory5> factory5;
	hres = factory4.As( &factory5 );
	ASSERT_HRES_IF_FAILED;

	hres = factory5->CheckFeatureSupport( DXGI_FEATURE_PRESENT_ALLOW_TEARING,
		&bAllowTearing,
		sizeof bAllowTearing );
	ASSERT_HRES_IF_FAILED;
	return SUCCEEDED( hres ) && bAllowTearing;
}
#endif

Graphics::Graphics( HWND hWnd,
	int width,
	int height )
	:
	m_width(width),
	m_height(height)
{
	HRESULT hres;
	auto& settings = g_settings.getSettings();
	if ( settings.bMultithreadedRendering )
	{
		m_deferredContexts.reserve( settings.nRenderingThreads );
		m_commandLists.reserve( settings.nRenderingThreads );
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1u;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = true;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // allow window resizing fullscreen/windowed etc.
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60u;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1u;
	swapChainDesc.SampleDesc.Count = 1u;
	swapChainDesc.SampleDesc.Quality = 0u;

	unsigned swapChainFlags = 0u;
#if defined _DEBUG && !defined NDEBUG
	swapChainFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	static const D3D_FEATURE_LEVEL acceptableFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	// create adapter, device, front|back buffers, swap chain and device rendering context
	createAdapters();
	const auto& primaryAdapter = m_adapters.front();
	hres = E_INVALIDARG;
#if defined _FLIP_PRESENT
	// make window-swap chain association
	makeWindowAssociationWithFactory( hWnd );
#else
	for ( DWORD i = 0u;
		hres == E_INVALIDARG || i < std::size( acceptableFeatureLevels ); ++i )
	{
		hres = D3D11CreateDeviceAndSwapChain( primaryAdapter.getAdapter(),
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			swapChainFlags,
			acceptableFeatureLevels,
			(unsigned) std::size( acceptableFeatureLevels ),
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&m_pSwapChain,
			&m_pDevice,
			&m_featureLevel,
			&m_pContext );
	}
#endif
	ASSERT_HRES_IF_FAILED;

#if defined _DEBUG && !defined NDEBUG
	// Check for DirectX Math library support
	if ( !dx::XMVerifyCPUSupport() )
	{
		MessageBoxW( hWnd,
			L"DirectX Math library support validation failure.",
			L"Error",
			MB_OK );
	}

	auto callMe = [this]()
	{
		interrogateDirectxFeatures();
	};
	std::call_once( ::g_startUpFlag,
		callMe );

	// create the info queue
	m_infoQueue = std::make_unique<DxgiInfoQueue>();
#endif

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pD3dBackBuffer;
	hres = m_pSwapChain->GetBuffer( 0u,
		__uuidof( ID3D11Texture2D ),
		&pD3dBackBuffer );
	ASSERT_HRES_IF_FAILED;

	m_globalColorBuffer = std::make_shared<RenderTargetOutput>( *this,
		pD3dBackBuffer.Get() );
	
	if constexpr ( GraphicsMode::get() == GraphicsMode::_2D )
	{// for d2d
		m_pCpuBuffer = static_cast<ColorBGRA*>( _aligned_malloc( sizeof( ColorBGRA )
			* width * height,
			16u ) );
	}

	m_fpsSpriteBatch = std::make_unique<dx::SpriteBatch>( m_pContext.Get() );
	m_spriteFont = std::make_unique<dx::SpriteFont>( m_pDevice.Get(),
		L"assets/fonts/myComicSansMSSpriteFont.spritefont" );
	
	// initialize Dear ImGui d3d11 Implementation
	if constexpr ( GraphicsMode::get() == GraphicsMode::_3D )
	{
		ImGui_ImplDX11_Init( m_pDevice.Get(),
			m_pContext.Get() );
	}

	m_fpsTimer.start();
}

Graphics::~Graphics()
{
	if constexpr ( GraphicsMode::get() == GraphicsMode::_3D )
	{
		ImGui_ImplDX11_Shutdown();
	}
	else
	{
		_aligned_free( m_pCpuBuffer );
		m_pCpuBuffer = nullptr;
	}
	m_pContext->ClearState();
#if defined _DEBUG && !defined NDEBUG
	d3d11DebugReport();
#endif
}

void Graphics::clearShaderSlots() noexcept
{
	// Clearing shader inputs to prevent simultaneous in/out binds carried over
	// from previous frame. Now we can start each frame with a clean slate
	// prevent OMSetRenderTargets State Hazard
	ID3D11ShaderResourceView* const pNullSrv = nullptr;
	// diffuse texture
	m_pContext->PSSetShaderResources( 0u,
		1u,
		&pNullSrv );
	DXGI_GET_QUEUE_INFO_GFX;
	// specular texture
	m_pContext->PSSetShaderResources( 1u,
		1u,
		&pNullSrv );
	DXGI_GET_QUEUE_INFO_GFX;
	// normal texture
	m_pContext->PSSetShaderResources( 2u,
		1u,
		&pNullSrv );
	DXGI_GET_QUEUE_INFO_GFX;
	// shadow map texture
	m_pContext->PSSetShaderResources( 3u,
		1u,
		&pNullSrv );
	DXGI_GET_QUEUE_INFO_GFX;
}

void Graphics::beginRendering() noexcept
{
	VTUNE_ITT_TASK_BEGIN( pStrIttBeginRendering );
	if constexpr ( GraphicsMode::get() == GraphicsMode::_3D )
	{// imgui new frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}
	else
	{
		static size_t cpuBuffer2dSize = m_width * m_height * sizeof ColorBGRA;
		memset( m_pCpuBuffer, 0u, cpuBuffer2dSize );
	}
	clearShaderSlots();
	VTUNE_ITT_TASK_END( pStrIttBeginRendering );
}

void Graphics::updateAndRenderFpsTimer()
{
	VTUNE_ITT_TASK_BEGIN( pStrIttFpsTimerRendering );
	static int fpsDisplayFrameCount = 0;
	static std::wstring fps;

	m_fpsSpriteBatch->Begin();
	auto& setMan = SettingsManager::getInstance();
	if ( setMan.getSettings().bFpsCounting )
	{
		++fpsDisplayFrameCount;

		// if more than 1000 ms have passed do an fps count
		if ( m_fpsTimer.getDurationFromStart() > 1000 )
		{
			fps = std::to_wstring( fpsDisplayFrameCount );
			OutputDebugStringW( fps.data() );
			m_fpsTimer.restart();
			fpsDisplayFrameCount = 0;
		}
	}

	// draw FPS text
	m_spriteFont->DrawString( m_fpsSpriteBatch.get(),
		fps.c_str(),
		dx::XMFLOAT2{0.0f, 0.0f},
		dx::Colors::Green,
		0.0f,
		dx::XMFLOAT2{0.0f, 0.0f},
		dx::XMFLOAT2{1.0f, 1.0f} );
	m_fpsSpriteBatch->End();
	VTUNE_ITT_TASK_END( pStrIttFpsTimerRendering );
}

void Graphics::endRendering()
{
	VTUNE_ITT_TASK_BEGIN( pStrIttEndRendering );
	if constexpr ( GraphicsMode::get() == GraphicsMode::_3D )
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
	}

#if defined _DEBUG && !defined NDEBUG
	m_infoQueue->markQueueIndex();
#endif

	HRESULT hres;
	if ( g_settings.getSettings().bVSync )
	{
#if defined _FLIP_PRESENT
		hres = m_pSwapChain->Present1( 1u,
			0u,
			 nullptr );
#else
		hres = m_pSwapChain->Present( 1u,
			0u );
#endif
	}
	else
	{
#if defined _FLIP_PRESENT
		hres = m_pSwapChain->Present1( 0u,
			0u,
			nullptr );
#else
		hres = m_pSwapChain->Present( 0u,
			0u );
#endif
	}

	if ( hres == DXGI_ERROR_DEVICE_REMOVED )
	{
		THROW_GRAPHICS_EXCEPTION( "Device Removed exception triggered!\nReason: "
			+ m_pDevice->GetDeviceRemovedReason() );
	}
	ASSERT_HRES_IF_FAILED;
	//m_pContext->ClearState();
	VTUNE_ITT_TASK_END( pStrIttEndRendering );
}

void Graphics::draw( unsigned count ) cond_noex
{
	m_pContext->Draw( count,
		0u );
	DXGI_GET_QUEUE_INFO_GFX;
}

void Graphics::drawIndexed( unsigned count ) cond_noex
{
	VTUNE_ITT_TASK_BEGIN( pStrIttDrawIndexed );
	if ( g_settings.getSettings().bMultithreadedRendering )
	{
		playbackDeferredCommandList();
	}
	else
	{
		m_pContext->DrawIndexed( count,
			0u,
			0u );
		DXGI_GET_QUEUE_INFO_GFX;
	}
	VTUNE_ITT_TASK_END( pStrIttDrawIndexed );
}

void Graphics::drawIndexedInstanced( unsigned indexCount,
	unsigned instanceCount ) cond_noex
{
	VTUNE_ITT_TASK_BEGIN( pStrIttDrawIndexedInstanced );
	if ( g_settings.getSettings().bMultithreadedRendering )
	{

	}
	else
	{
		//m_pContext->DrawIndexedInstanced();
	}
	DXGI_GET_QUEUE_INFO_GFX;
	VTUNE_ITT_TASK_END( pStrIttDrawIndexedInstanced );
}


ColorBGRA*& Graphics::getCpuBuffer()
{
	return m_pCpuBuffer;
}

void Graphics::setViewMatrix( const dx::XMMATRIX& cam ) noexcept
{
	m_view = cam;
}

void Graphics::setProjectionMatrix( const dx::XMMATRIX& proj ) noexcept
{
	m_projection = proj;
}

dx::XMMATRIX Graphics::getViewMatrix() const noexcept
{
	return m_view;
}

dx::XMMATRIX Graphics::getProjectionMatrix() const noexcept
{
	return m_projection;
}

unsigned Graphics::getClientWidth() const noexcept
{
	return m_width;
}

unsigned Graphics::getClientHeight() const noexcept
{
	return m_height;
}

IRenderTargetView* Graphics::getRenderTarget() const noexcept
{
	return m_globalColorBuffer.get();
}

std::shared_ptr<IRenderTargetView> Graphics::shareRenderTarget()
{
	return m_globalColorBuffer;
}

#if defined _DEBUG && !defined NDEBUG
DxgiInfoQueue& Graphics::getInfoQueue() const noexcept
{
	return *( m_infoQueue.get() );
}
#endif

void Graphics::createAdapters()
{
	HRESULT hres = CreateDXGIFactory1( __uuidof( IDXGIFactory1 ),
		reinterpret_cast<void**>( m_pFactory.GetAddressOf() ) );
	ASSERT_HRES_IF_FAILED;

	IDXGIAdapter1* pAdapter = nullptr;
	unsigned adapterIndex = 0;
	while ( SUCCEEDED( m_pFactory->EnumAdapters1( adapterIndex,
		&pAdapter ) ) )
	{
		m_adapters.emplace_back( pAdapter );
		adapterIndex += 1;
	}
}

std::vector<Graphics::Adapter>& Graphics::getAdapters() const
{
	if ( m_adapters.empty() )
	{
		THROW_GRAPHICS_EXCEPTION( "No adapter set!" );
	}
	return m_adapters;
}

// probably should call this when the Model is being loaded not when pass->run() -> Job->run()
void Graphics::recordDeferredCommandList()
{
	//HRESULT hres;
	//hres = m_pDevice->CreateDeferredContext( 0, &pDeferredContexts );
	//// Use the deferred context to render:
	//float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	//pDeferredContexts->ClearRenderTargetView( pRenderTargetView, ClearColor );
	//// Add additional rendering commands
	//// ...
	//hres = pDeferredContexts->FinishCommandList( FALSE,
	//	&pCommandList);
}

void Graphics::playbackDeferredCommandList()
{
	//if( pCommandList )
	//{
	//	m_pContext->ExecuteCommandList( pCommandList,
	//		TRUE );
	//}
}

#if defined _DEBUG && !defined NDEBUG
void Graphics::interrogateDirectxFeatures()
{
	// threading
	D3D11_FEATURE_DATA_THREADING threadingInfo;
	ZeroMemory( &threadingInfo,
		sizeof( D3D11_FEATURE_DATA_THREADING ) );
	HRESULT hres = m_pDevice->CheckFeatureSupport( D3D11_FEATURE::D3D11_FEATURE_THREADING,
		&threadingInfo,
		sizeof( D3D11_FEATURE_DATA_THREADING ) );
	ASSERT_HRES_IF_FAILED;

	using namespace std::string_literals;
	auto& console = KeyConsole::getInstance();
	console.log( threadingInfo.DriverConcurrentCreates ? 
		"Resources can be created concurrently on multiple threads.\n"s :
		"No DirectX concurrency possible\n"s );

	console.log( threadingInfo.DriverCommandLists ?
		"Command lists are supported by the current driver.\n"s :
		"Commands lists will be emulated in software.\n"s );

	D3D11_FEATURE_DATA_D3D11_OPTIONS hwopts11{};
	hres = m_pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D11_OPTIONS,
		&hwopts11,
		sizeof( hwopts11 ) );
	ASSERT_HRES_IF_FAILED;

	if ( !hwopts11.MapNoOverwriteOnDynamicConstantBuffer )
	{
		console.log( "Constant Buffer D3D11_MAP_WRITE_NO_OVERWRITE unsupported!\n"s );
	}
	if ( !hwopts11.MapNoOverwriteOnDynamicBufferSRV )
	{
		console.log( "Shader Resource View D3D11_MAP_WRITE_NO_OVERWRITE unsupported!\n"s );
	}

	unsigned formatSupport;
	hres = m_pDevice->CheckFormatSupport( DXGI_FORMAT_B8G8R8A8_UNORM,
		&formatSupport );
	ASSERT_HRES_IF_FAILED;
}

void Graphics::d3d11DebugReport()
{
	HRESULT hres;
	hres = m_pDevice->QueryInterface( __uuidof( ID3D11Debug ),
		reinterpret_cast<void**>( &m_pDebug ) );
	ASSERT_HRES_IF_FAILED;

	// report any live objects
	if ( m_pDebug != nullptr )
	{
		OutputDebugStringW( L"\nReporting Debug Objects\n" );
		hres = m_pDebug->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
		ASSERT_HRES_IF_FAILED;
		m_pDebug = nullptr;
	}

	ATL::CComPtr<ID3DUserDefinedAnnotation> pUserDefinedAnnotation;
	hres = m_pContext->QueryInterface( IID_PPV_ARGS( &pUserDefinedAnnotation ) );
	ASSERT_HRES_IF_FAILED;
}
#endif

ColorBGRA Graphics::getPixel( int x,
	int y ) const noexcept
{
	ASSERT( x >= 0, "!( x >= 0 )" );
	ASSERT( x < m_width, "!( x < width )" );
	ASSERT( y >= 0, "!( y >= 0)" );
	ASSERT( y < m_height, "!( y < height )" );
	return m_pCpuBuffer[m_width * y + x];
}

void Graphics::putPixel( int x,
	int y,
	ColorBGRA col )
{
	ASSERT( x >= 0, "!( x >= 0 )" );
	ASSERT( x < m_width, "!( x < width )" );
	ASSERT( y >= 0, "!( y >= 0)" );
	ASSERT( y < m_height, "!( y < height )" );	
	m_pCpuBuffer[m_width * y + x] = col;
}

void Graphics::drawRect( int x0,
	int y0,
	int x1,
	int y1,
	ColorBGRA col )
{
	if ( x0 > x1 )
	{
		std::swap( x0,
			x1 );
	}
	if ( y0 > y1 )
	{
		std::swap( y0,
			y1 );
	}

	for ( int x = x0; x < x1; ++x )
	{
		for ( int y = y0; y < y1; ++y )
		{
			putPixel( x,
				y,
				col );
		}
	}
}

void Graphics::drawTriangle()
{
	//// line for x1, y1, x2, y2
	//line(150, 150, 450, 150);
	//// line for x1, y1, x2, y2
	//line(150, 150, 300, 300);
	//// line for x1, y1, x2, y2
	//line(450, 150, 300, 300);
}

void Graphics::drawLine( int x0,
	int x1,
	int y0,
	int y1,
	ColorBGRA col )
{
	float m = 0.0f;
	if ( x1 != x0 )
	{
		m = ( y1 - y0 ) / ( x1 - x0 );
	}

	if ( x1 != x0 && std::abs( m ) <= 1.0f )
	{// not a vertical line
		if ( x0 > x1 )
		{
			std::swap( x0, x1);
			std::swap( y0, y1);
		}

		const float b = y0 - m * x0;
		for ( int x = x0; x <= x1; ++x )
		{
			const float y = m * (float)x + b;
			const int yi = y;
			if ( x >= 0 && x < m_width && yi >= 0 && yi < m_height )
			{
				putPixel( x,
					yi,
					col );
			}
		}
	}
	else
	{
		if ( y0 > y1 )
		{
			std::swap( x0, x1);
			std::swap( y0, y1);
		}

		const float w = ( x1 - x0 ) / ( y1 - y0 );
		const float p = x0 - w * y0;
		for ( int y = y0; y <= y1; ++y )
		{
			const float x = w * (float)y + p;
			const int xi = x;
			if ( xi >= 0 && xi < m_width && y >= 0 && y < m_height )
			{
				putPixel( xi,
					y,
					col );
			}
		}
	}
}

void Graphics::drawCircle( int centerX,
	int centerY,
	int radius,
	ColorBGRA col )
{
	int radiusSquared = radius * radius;
	int yDiff;
	int xDiff;
	// [-radius, radius]
	for ( int yLoop = centerY - radius; yLoop <= centerY + radius; ++yLoop )
	{
		for ( int xLoop = centerX - radius; xLoop <= centerX + radius; ++xLoop )
		{
			xDiff = centerX - xLoop;
			yDiff = centerY - yLoop;
			if ( xDiff * xDiff + yDiff * yDiff <= radiusSquared )
			{
				putPixel( xLoop,
					yLoop,
					col );
			}
		}
	}
}

std::vector<DirectX::XMFLOAT2> Graphics::drawStar( float outerRadius,
	float innerRadius,
	int nFlares )
{
	std::vector<dx::XMFLOAT2> star;
	star.reserve( nFlares * 2 );
	const float dTheta = 2.0f * 3.14159f / float( nFlares * 2 );
	for ( int i = 0; i < nFlares * 2; ++i )
	{
		const float rad = ( i % 2 == 0 ) ?
			outerRadius :
			innerRadius;
		star.emplace_back( rad * cos( float(i) * dTheta ), rad * sin( float(i) * dTheta ) );
	}
	return star;
}

//drawBezier
// general implementation for a curve with any number of points.
/*
putPixel( getBezierPoint() )
vec2 getBezierPoint( vec2* points, int numPoints, float t ) {
	vec2* tmp = new vec2[numPoints];
	memcpy(tmp, points, numPoints * sizeof(vec2));
	int i = numPoints - 1;
	while (i > 0) {
		for (int k = 0; k < i; k++)
			tmp[k] = tmp[k] + t * ( tmp[k+1] - tmp[k] );
		i--;
	}
	vec2 answer = tmp[0];
	delete[] tmp;
	return answer;
}
*/

/*
bool raySphereIntersect( D3DXVECTOR3 rayOrigin, D3DXVECTOR3 rayDirection, float radius)
{
	float a, b, c, discriminant;


	// Calculate the a, b, and c coefficients.
	a = (rayDirection.x * rayDirection.x) + (rayDirection.y * rayDirection.y) + (rayDirection.z * rayDirection.z);
	b = ((rayDirection.x * rayOrigin.x) + (rayDirection.y * rayOrigin.y) + (rayDirection.z * rayOrigin.z)) * 2.0f;
	c = ((rayOrigin.x * rayOrigin.x) + (rayOrigin.y * rayOrigin.y) + (rayOrigin.z * rayOrigin.z)) - (radius * radius);

	// Find the discriminant.
	discriminant = (b * b) - (4 * a * c);

	// if discriminant is negative the picking ray missed the sphere, otherwise it intersected the sphere.
	if (discriminant < 0.0f)
	{
		return false;
	}

	return true;
}

	private Vector3f getPointOnRay(Vector3f ray, float distance) {
	Vector3f camPos = camera.getPosition();
	Vector3f start = new Vector3f(camPos.x, camPos.y, camPos.z);
	Vector3f scaledRay = new Vector3f(ray.x * distance, ray.y * distance, ray.z * distance);
	return Vector3f.add(start, scaledRay, null);
}

private Vector3f binarySearch(int count, float start, float finish, Vector3f ray) {
	float half = start + ((finish - start) / 2f);
	if (count >= RECURSION_COUNT) {
		Vector3f endPoint = getPointOnRay(ray, half);
		Terrain terrain = getTerrain(endPoint.getX(), endPoint.getZ());
		if (terrain != null) {
			return endPoint;
		} else {
			return null;
		}
	}
	if (intersectionInRange(start, half, ray)) {
		return binarySearch(count + 1, start, half, ray);
	} else {
		return binarySearch(count + 1, half, finish, ray);
	}
}

private boolean intersectionInRange(float start, float finish, Vector3f ray) {
	Vector3f startPoint = getPointOnRay(ray, start);
	Vector3f endPoint = getPointOnRay(ray, finish);
	if (!isUnderGround(startPoint) && isUnderGround(endPoint)) {
		return true;
	} else {
		return false;
	}
}

private boolean isUnderGround(Vector3f testPoint) {
	Terrain terrain = getTerrain(testPoint.getX(), testPoint.getZ());
	float height = 0;
	if (terrain != null) {
		height = terrain.getHeightOfTerrain(testPoint.getX(), testPoint.getZ());
	}
	if (testPoint.y < height) {
		return true;
	} else {
		return false;
	}
}*/

Graphics::GraphicsException::GraphicsException( int line,
	const char* file,
	const char* function,
	const std::string &msg ) noexcept
	:
	KeyException{line, file, function, msg}
{

}

const std::string Graphics::GraphicsException::getType() const noexcept
{
	return typeid( *this ).name();
}

const char* Graphics::GraphicsException::what() const noexcept
{
	return KeyException::what();
}