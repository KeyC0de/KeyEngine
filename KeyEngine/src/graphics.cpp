#include <sstream>
#include <mutex>
#include "dxgi1_5.h"
#include "graphics.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui_impl_dx11.h"
#	include "imgui/imgui_impl_win32.h"
#endif // !FINAL_RELEASE
#include "render_target.h"
#include "depth_stencil_view.h"
#include "reporter_access.h"
#include "reporter_listener_events.h"
#include "utils.h"
#include "os_utils.h"
#include "settings_manager.h"
#include "console.h"
#include "graphics_mode.h"
#include "rectangle.h"
#include "vtune_itt_domain.h"
#include "texture.h"
#include "math_utils.h"
#include "renderer.h"
#include "camera_manager.h"
#include "camera.h"

#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#ifdef D2D_INTEROP
#	pragma comment( lib, "d2d1.lib" )
#	pragma comment( lib, "dwrite.lib" )
#endif


namespace mwrl = Microsoft::WRL;
namespace dx = DirectX;

// create ITT string handles
#ifdef _PROFILE
static __itt_string_handle *pStrIttDrawIndexed = __itt_string_handle_create( L"DrawIndexed" );
static __itt_string_handle *pStrIttDrawIndexedInstanced = __itt_string_handle_create( L"DrawIndexedInstanced" );
static __itt_string_handle *pStrIttBeginRendering = __itt_string_handle_create( L"BeginRendering" );
static __itt_string_handle *pStrIttFpsTimerRendering = __itt_string_handle_create( L"FpsTimerRendering" );
static __itt_string_handle *pStrIttEndRendering = __itt_string_handle_create( L"EndRendering" );
#endif

Graphics::Adapter::Adapter( IDXGIAdapter *pAdapter )
{
	m_pAdapter = pAdapter;
	pAdapter->GetDesc( &m_desc );

#if defined _DEBUG && !defined NDEBUG
	const char *primaryAdapterName = "PrimaryDxgiAdapter";
	m_pAdapter->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( primaryAdapterName ), primaryAdapterName );
#endif
}

const DXGI_ADAPTER_DESC* Graphics::Adapter::getDesc() const noexcept
{
	return &m_desc;
}

IDXGIAdapter* Graphics::Adapter::getAdapter() const noexcept
{
	return m_pAdapter.Get();
}

void Graphics::Adapter::getVRamDetails() const noexcept
{
	const auto desc = getDesc();

	const std::string description = util::ws2s( desc->Description );
	const INT64 vRam = desc->DedicatedVideoMemory;
	const INT64 ram = desc->DedicatedSystemMemory;
	const INT64 sharedRam = desc->SharedSystemMemory;
	const std::string featureLevel{ENUM_STR( Graphics::s_featureLevel )};

#if defined _DEBUG && !defined NDEBUG
	KeyConsole& console = KeyConsole::getInstance();
	console.print( "***************** GRAPHICS ADAPTER DETAILS ***********************" );
	console.print( "Adapter Description: " + description );
	console.print( "Dedicated Video RAM: " + std::to_string( vRam ) );
	console.print( "Dedicated System RAM: " + std::to_string( ram ) );
	console.print( "Shared System RAM: " + std::to_string( sharedRam ) );
	console.print( "Feature Level: " + featureLevel );
#endif
}

void Graphics::makeWindowAssociationWithFactory( HWND hWnd,
	UINT flags )
{
#if defined FLIP_PRESENT
	HRESULT hres = m_pDxgiFactory->MakeWindowAssociation( hWnd, flags );
	ASSERT_HRES_IF_FAILED;
#endif
}

Graphics::Graphics( const HWND hWnd,
	const int width,
	const int height,
	const MultisamplingMode multisamplingMode /*= MultisamplingMode::None*/ )
	:
	m_width(width),
	m_height(height),
	m_hParentWnd{hWnd},
	m_swapChainFlags{0u}
{
	HRESULT hres;
	auto &settings = SettingsManager::getInstance().getSettings();
	if ( settings.bMultithreadedRendering )
	{
		m_deferredContexts.reserve( settings.nRenderingThreads );
		m_commandLists.reserve( settings.nRenderingThreads );
	}

	// create factory, adapter, device, device rendering context(s), front|back buffers swap chain, output device
	createFactory();
	createAdapters();
	const auto &primaryAdapter = s_adapters.front();
	hres = E_INVALIDARG;

	m_swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// allows switching between fullscreen mode
#if defined _DEBUG && !defined NDEBUG
	m_swapChainFlags |= D3D11_CREATE_DEVICE_DEBUG;
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

#ifdef FLIP_PRESENT
	if ( checkTearingSupport() )
	{
		m_swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		m_presentFlags = DXGI_PRESENT_ALLOW_TEARING;
	}

	for ( DWORD i = 0u; hres == E_INVALIDARG || i < std::size( acceptableFeatureLevels ); ++i )
	{
		hres = D3D11CreateDevice( primaryAdapter.getAdapter(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, m_swapChainFlags, acceptableFeatureLevels, (unsigned) std::size( acceptableFeatureLevels ), D3D11_SDK_VERSION, &m_pDevice, &s_featureLevel, &m_pImmediateContext );
	}
	ASSERT_HRES_IF_FAILED;

#if defined _DEBUG && !defined NDEBUG
	const char *deviceName = "KeyDevice";
	m_pDevice->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( deviceName ), deviceName );

	const char *immediateContextName = "ImmediateContext";
	m_pImmediateContext->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( immediateContextName ), immediateContextName );
#endif

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2u;
	swapChainDesc.Stereo = false;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1u;	// MSAA not supported in FLIP mode for the back buffer
	swapChainDesc.SampleDesc.Quality = 0u;
	swapChainDesc.Flags = m_swapChainFlags;

	hres = m_pDxgiFactory->CreateSwapChainForHwnd( m_pDevice.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &m_pSwapChain );
	ASSERT_HRES_IF_FAILED;

	// make window-swap chain association
	makeWindowAssociationWithFactory( hWnd, DXGI_MWA_NO_WINDOW_CHANGES );
#else
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1u;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = true;
	swapChainDesc.Flags = m_swapChainFlags;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60u;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1u;
	if ( multisamplingMode == MultisamplingMode::None )
	{
		swapChainDesc.SampleDesc.Count = 1u;
		swapChainDesc.SampleDesc.Quality = 0u;
	}
	else
	{
		// swapChainDesc.SampleDesc.Count = 1u;		// #TODO: multisampling
		// swapChainDesc.SampleDesc.Quality = 0u;
	}

	for ( DWORD i = 0u; hres == E_INVALIDARG || i < std::size( acceptableFeatureLevels ); ++i )
	{
		hres = D3D11CreateDeviceAndSwapChain( primaryAdapter.getAdapter(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, m_swapChainFlags, acceptableFeatureLevels, (unsigned) std::size( acceptableFeatureLevels ), D3D11_SDK_VERSION, &swapChainDesc, &m_pSwapChain, &m_pDevice, &s_featureLevel, &m_pImmediateContext );
	}
	ASSERT_HRES_IF_FAILED;
#endif

	ASSERT( s_featureLevel == D3D_FEATURE_LEVEL_11_1, "Old feature level!" );

#if defined _DEBUG && !defined NDEBUG
	interrogateDirectxFeatures();
#endif

	setupMonitors();

	// Create Renderer
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		m_pRenderer = std::make_unique<ren::Renderer3d>( *this, true, 4, 3.0f );
		m_pRenderer3d = dynamic_cast<ren::Renderer3d*>( m_pRenderer.get() );
	}
	else
	{
		m_pRenderer = std::make_unique<ren::Renderer2d>( *this );
		m_pRenderer2d = dynamic_cast<ren::Renderer2d*>( m_pRenderer.get() );
	}

	// Create the rest
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
#ifndef FINAL_RELEASE
		// initialize Dear ImGui d3d11 Implementation
		ImGui_ImplDX11_Init( m_pDevice.Get(), m_pImmediateContext.Get() );
#endif
	}
	else
	{
		m_pCpuBuffer = static_cast<ColorBGRA*>( _aligned_malloc( sizeof( ColorBGRA ) * width * height, 16u ) );
	}

	if ( settings.bVSync )
	{
		settings.iMaxFps = util::round( getRefreshRate() );
	}

	m_fpsTimer.start();
}

Graphics::~Graphics()
{
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
#ifndef FINAL_RELEASE
		ImGui_ImplDX11_Shutdown();
#endif
	}
	else
	{
		_aligned_free( m_pCpuBuffer );
		m_pCpuBuffer = nullptr;
	}
	cleanState();
	d3d11DebugReport();
}

void Graphics::clearShaderSlots() noexcept
{
	// Clearing shader input slots to prevent simultaneous in/out binds carried over from previous frame.
	// This is to prevent OMSetRenderTargets state hazard.
	ID3D11ShaderResourceView *const pNullSrv = nullptr;

	// diffuse texture
	m_pImmediateContext->PSSetShaderResources( 0u, 1u, &pNullSrv );
	DXGI_GET_QUEUE_INFO_GFX;

	// specular texture
	m_pImmediateContext->PSSetShaderResources( 1u, 1u, &pNullSrv );
	DXGI_GET_QUEUE_INFO_GFX;

	// normal texture
	m_pImmediateContext->PSSetShaderResources( 2u, 1u, &pNullSrv );
	DXGI_GET_QUEUE_INFO_GFX;

	// shadow map texture
	m_pImmediateContext->PSSetShaderResources( 3u, 1u, &pNullSrv );

	// post processing texture
	m_pImmediateContext->PSSetShaderResources( 4u, 1u, &pNullSrv );
	DXGI_GET_QUEUE_INFO_GFX;
}

void Graphics::resize( unsigned newWidth,
	unsigned newHeight )
{
	auto &caman = CameraManager::getInstance();
	caman.getActiveCamera().setTethered( true );		// prevent camera movement while resizing, #TODO: instead change state to MenuState

	HRESULT hres;
	if ( newWidth == 0 && newHeight == 0 )
	{
		m_bFullscreenMode = true;
		newWidth = GetSystemMetrics( SM_CXSCREEN );
		newHeight = GetSystemMetrics( SM_CYSCREEN );
	}
	else
	{
		m_bFullscreenMode = false;
	}

	m_width = newWidth;
	m_height = newHeight;

	// make a fullscreen transition if necessary; must be called before ResizeBuffers
	hres = m_pSwapChain->SetFullscreenState( m_bFullscreenMode ? TRUE : FALSE, m_bFullscreenMode ? m_pDxgiOutput.Get() : nullptr );
	ASSERT_HRES_IF_FAILED;

	// assemble all bound RTV & DSVs and clear them & empty their shared_ptrs
	if ( m_pBackBufferRtv.use_count() > 0 )
	{
		m_pBackBufferRtv->clean( *this );
	}
	if ( m_pBackBufferDsv.use_count() > 0 )
	{
		m_pBackBufferDsv->clean( *this );
	}
	if ( m_pOffscreenRtv->rtv().use_count() > 0 )
	{
		m_pOffscreenRtv->rtv()->clean( *this );
	}
	if ( m_pOffscreenDsv->dsv().use_count() > 0 )
	{
		m_pOffscreenDsv->dsv()->clean( *this );
	}

	m_pBackBufferRtv.reset();
	m_pBackBufferDsv.reset();
	m_pOffscreenRtv->rtv().reset();
	m_pOffscreenRtv.reset();
	m_pOffscreenDsv->dsv().reset();
	m_pOffscreenDsv.reset();

	// resize the RTV:
	hres = m_pSwapChain->ResizeBuffers( 0u, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, m_swapChainFlags );
	ASSERT_HRES_IF_FAILED;
	// may need to handle DXGI_ERROR_DEVICE_REMOVED & DXGI_ERROR_DEVICE_RESET

	// recreate the RTVs & DSVs:
	getRenderTargetFromBackBuffer();
	getDepthBufferFromBackBuffer();
	getRenderTargetOffscreen( 0u, RenderTargetViewMode::DefaultRT );
	getDepthBufferOffscreen( 0u, DepthStencilViewMode::DefaultDS );

#if defined _DEBUG && !defined NDEBUG
	// Assert that window width/height is equal to swap-chain width/height
	DXGI_SWAP_CHAIN_DESC desc;
	POD_ZERO( desc );
	m_pSwapChain->GetDesc( &desc );
	ASSERT( desc.BufferDesc.Width == newWidth && desc.BufferDesc.Height == newHeight, "Resizing malfunction!" );
#endif

	m_pRenderer->recreate( *this );
	auto &reportingNexus = ReportingNexus::getInstance();
	static_cast<IReporter<SwapChainResized>&>( reportingNexus ).notifyListeners( SwapChainResized{} );

	// update cameras
	CameraManager::getInstance().updateDimensions( *this );
	caman.getActiveCamera().setTethered( false );
}

void Graphics::setupMonitors()
{
	auto *pMainAdapter = s_adapters[0].getAdapter();

	// enumerate adapter outputs
	IDXGIOutput* output;
	unsigned i = 0;
	while ( pMainAdapter->EnumOutputs( i, &output ) != DXGI_ERROR_NOT_FOUND )
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc( &desc );
		++i;
	}

	if ( i > 1 )
	{
		THROW_GRAPHICS_EXCEPTION( "Multiple monitors are not currently supported. Please set only 1 monitor per graphics card for use in this application." );
	}

	HRESULT hres;
	mwrl::ComPtr<IDXGIOutput> dxgiOutput;
	hres = m_pSwapChain->GetContainingOutput( &dxgiOutput );
	ASSERT_HRES_IF_FAILED;

	hres = dxgiOutput.As( &m_pDxgiOutput );
	ASSERT_HRES_IF_FAILED;

#if defined _DEBUG && !defined NDEBUG
	const char *primaryOutputMonitor = "OutputMonitorPrimary";
	m_pDxgiOutput->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( primaryOutputMonitor ), primaryOutputMonitor );
#endif
}

double Graphics::getRefreshRate() const noexcept
{
	HRESULT hres;
	DXGI_MODE_DESC1 emptyMode{};
	DXGI_MODE_DESC1 modeDesc;
	hres = m_pDxgiOutput->FindClosestMatchingMode1( &emptyMode, &modeDesc, m_pDevice.Get() );
	ASSERT_HRES_IF_FAILED;

	return static_cast<double>( modeDesc.RefreshRate.Numerator ) / static_cast<double>( modeDesc.RefreshRate.Denominator );
}

size_t Graphics::getFrameNum() const noexcept
{
	return m_currentFrame;
}

void Graphics::runRenderer() noexcept
{
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		m_pRenderer3d->run( *this );
	}
	else
	{
		m_pRenderer2d->run( *this );
	}
}

ren::Renderer& Graphics::getRenderer() noexcept
{
	return *m_pRenderer;
}

ren::Renderer3d& Graphics::getRenderer3d() noexcept
{
	ASSERT( m_pRenderer3d, "3d Renderer not in place!" );
	return *m_pRenderer3d;
}

ren::Renderer2d& Graphics::getRenderer2d() noexcept
{
	ASSERT( m_pRenderer2d, "2d Renderer not in place!" );
	return *m_pRenderer2d;
}

void Graphics::cleanState() noexcept
{
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		clearShaderSlots();
		m_pImmediateContext->ClearState();	// release all references
		for ( auto dc : m_commandLists )
		{
			if ( dc )
			{
				m_pImmediateContext->FinishCommandList( FALSE, &dc );
				dc->Release();
			}
		}
		m_pImmediateContext->Flush();		// flush any remaining commands
	}
	else
	{
#ifdef D2D_INTEROP
		m_p2DContext->Flush();
#endif
	}
}

void Graphics::profile() const noexcept
{
#ifdef _PROFILE
	HRESULT hres;
	unsigned nPresents = -1;
	hres = m_pSwapChain->GetLastPresentCount( &nPresents );
	ASSERT_HRES_IF_FAILED;

	KeyConsole& console = KeyConsole::getInstance();
	console.print( "Present counts = " + std::to_string( nPresents ) );

	// can only be used on flip mode swap chains or when fullscreen exclusive mode is enabled
	DXGI_FRAME_STATISTICS frameStats;
	hres = m_pSwapChain->GetFrameStatistics( &frameStats );
	ASSERT_HRES_IF_FAILED;

	// query frameStats for stuff
#endif
}

void Graphics::beginFrame() noexcept
{
	++m_currentFrame;
	PROFILE_VTUNE_ITT_TASK_BEGIN( pStrIttBeginRendering );
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
#ifndef FINAL_RELEASE
		// imgui new frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
#endif
		clearShaderSlots();
#ifdef D2D_INTEROP
		begin2dDraw();
#endif
	}
	else
	{
		static size_t cpuBuffer2dSize = (size_t)m_width * m_height * sizeof ColorBGRA;
		memset( m_pCpuBuffer, 0u, cpuBuffer2dSize );
	}
	PROFILE_VTUNE_ITT_TASK_END;
}

void Graphics::endFrame()
{
	m_pRenderer->reset();
	PROFILE_VTUNE_ITT_TASK_BEGIN( pStrIttEndRendering );
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
#ifndef FINAL_RELEASE
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
#endif

#ifdef D2D_INTEROP
		end2dDraw();
#endif
	}
	present();
	PROFILE_VTUNE_ITT_TASK_END;
}

void Graphics::present()
{
	HRESULT hres;
	SettingsManager &settings = SettingsManager::getInstance();
	const bool hasVsync = settings.getSettings().bVSync;

	//if ( settings.iMaxFps > 0 )
	//{
	//	// #TODO: enable frame limiter
	//	const float waitForMs = minFrameTimeMs - frameTime;
	//	if ( waitForMs > 0.0f )
	//	{
	//		m_gameTimer.delayFor( waitForMs );
	//	}
	//}

#if defined FLIP_PRESENT
	DXGI_PRESENT_PARAMETERS presentParams;
	POD_ZERO( presentParams );
	// #FIXME: setting VSync off during flip model doesn't make it so
	hres = m_pSwapChain->Present1( hasVsync ? settings.getSettings().iPresentInterval : 0u, m_presentFlags, &presentParams );
#else
	hres = m_pSwapChain->Present( hasVsync ? 1u : 0u, 0u );
#endif
	DXGI_GET_QUEUE_INFO_GFX;

#if defined _DEBUG && !defined NDEBUG
	if ( hres == DXGI_ERROR_DEVICE_REMOVED )
	{
		THROW_GRAPHICS_EXCEPTION( "Device Removed exception triggered!\nReason: " + m_pDevice->GetDeviceRemovedReason() );
	}
	else if ( hres == DXGI_ERROR_INVALID_CALL )
	{
		THROW_GRAPHICS_EXCEPTION( "Invalid Presentation parameters!" );
	}
#endif

	ASSERT_HRES_IF_FAILED;
}

void Graphics::draw( const unsigned count ) cond_noex
{
	m_pImmediateContext->Draw( count, 0u );
	DXGI_GET_QUEUE_INFO_GFX;
}

void Graphics::drawIndexed( const unsigned count ) cond_noex
{
	PROFILE_VTUNE_ITT_TASK_BEGIN( pStrIttDrawIndexed );
	SettingsManager &setMan = SettingsManager::getInstance();
	if ( setMan.getSettings().bMultithreadedRendering )
	{
		playbackDeferredCommandList();
	}
	else
	{
		m_pImmediateContext->DrawIndexed( count, 0u, 0u );
	}
	DXGI_GET_QUEUE_INFO_GFX;
	PROFILE_VTUNE_ITT_TASK_END;
}

void Graphics::drawIndexedInstanced( const unsigned indexCount,
	const unsigned instanceCount ) cond_noex
{
	PROFILE_VTUNE_ITT_TASK_BEGIN( pStrIttDrawIndexedInstanced );
	SettingsManager &setMan = SettingsManager::getInstance();
	if ( setMan.getSettings().bMultithreadedRendering )
	{

	}
	else
	{
		//m_pImmediateContext->DrawIndexedInstanced();
	}
	DXGI_GET_QUEUE_INFO_GFX;
	PROFILE_VTUNE_ITT_TASK_END;
}


ColorBGRA*& Graphics::cpuBuffer()
{
	return m_pCpuBuffer;
}

void Graphics::setViewMatrix( const dx::XMMATRIX &cam ) noexcept
{
	m_view = cam;
}

void Graphics::setProjectionMatrix( const dx::XMMATRIX &proj ) noexcept
{
	m_projection = proj;
}

const dx::XMMATRIX& Graphics::getViewMatrix() const noexcept
{
	return m_view;
}

const dx::XMMATRIX& Graphics::getProjectionMatrix() const noexcept
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

std::shared_ptr<RenderTargetOutput> Graphics::getRenderTargetFromBackBuffer()
{
	if ( !m_pBackBufferRtv )
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pD3dBackBufferTex;
		HRESULT hres = m_pSwapChain->GetBuffer( 0u, __uuidof( ID3D11Texture2D ), &pD3dBackBufferTex );
		ASSERT_HRES_IF_FAILED;

		m_pBackBufferRtv = std::make_shared<RenderTargetOutput>( *this, pD3dBackBufferTex.Get() );
#if defined _DEBUG && !defined NDEBUG
		m_pBackBufferRtv->setDebugObjectName( "BackBufferRenderTargetView" );
#endif
	}
	return m_pBackBufferRtv;
}

std::shared_ptr<DepthStencilOutput> Graphics::getDepthBufferFromBackBuffer()
{
	if ( !m_pBackBufferDsv )
	{
		m_pBackBufferDsv = std::make_shared<DepthStencilOutput>( *this );
#if defined _DEBUG && !defined NDEBUG
		m_pBackBufferDsv->setDebugObjectName( "BackBufferDepthStencilView" );
#endif
	}
	return m_pBackBufferDsv;
}

std::shared_ptr<TextureOffscreenRT> Graphics::getRenderTargetOffscreen( const unsigned slot,
	const RenderTargetViewMode rtvMode )
{
	if ( !m_pOffscreenRtv )
	{
		m_pOffscreenRtv = std::make_unique<TextureOffscreenRT>( *this, m_width, m_height, slot, rtvMode );
#if defined _DEBUG && !defined NDEBUG
		const char *offscreenRtv = "OffscreenRenderTargetTextureView1";
		m_pOffscreenRtv->rtv()->d3dResource()->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( offscreenRtv ), offscreenRtv );
#endif
	}
	return m_pOffscreenRtv;
}

std::shared_ptr<TextureOffscreenDS> Graphics::getDepthBufferOffscreen( const unsigned slot,
	const DepthStencilViewMode dsvMode )
{
	if ( !m_pOffscreenDsv )
	{
		m_pOffscreenDsv = std::make_unique<TextureOffscreenDS>( *this, m_width, m_height, slot, dsvMode );
#if defined _DEBUG && !defined NDEBUG
		const char *offscreenDsv = "OffscreenDepthStencilTextureView1";
		m_pOffscreenDsv->dsv()->d3dResource()->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( offscreenDsv ), offscreenDsv );
#endif
	}
	return m_pOffscreenDsv;
}

void Graphics::bindBackBufferAsInput()	// #UNUSED
{
	mwrl::ComPtr<ID3D11Texture2D> pD3dBackBufferTex;
	HRESULT hres = m_pSwapChain->GetBuffer( 0u, __uuidof( ID3D11Texture2D ), &pD3dBackBufferTex );

	D3D11_TEXTURE2D_DESC pD3dBackBufferTexDesc{};
	pD3dBackBufferTex->GetDesc( &pD3dBackBufferTexDesc );
	if ( !util::isSetByNumber( pD3dBackBufferTexDesc.BindFlags, D3D11_BIND_SHADER_RESOURCE ) )		// requires SwapChain's desc BufferUsage to have `DXGI_USAGE_SHADER_INPUT` flag set
	{
		THROW_GRAPHICS_EXCEPTION( "You cannot bind the back buffer texture as Input to the pipeline! If you are using pre-D3D12 API use a different bufferUsage - like `DXGI_USAGE_SHADER_INPUT`" );
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC backBufferSrvDesc = {};
	backBufferSrvDesc.Format = pD3dBackBufferTexDesc.Format;
	backBufferSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	backBufferSrvDesc.Texture2D.MostDetailedMip = 0u;
	backBufferSrvDesc.Texture2D.MipLevels = 1u;

	mwrl::ComPtr<ID3D11ShaderResourceView> pBackBufferSrv;
	m_pDevice->CreateShaderResourceView( pD3dBackBufferTex.Get(), &backBufferSrvDesc, &pBackBufferSrv );

	IRenderTargetView::unbind( *this );

	// now bind to slot 0 the back buffer srv
	m_pImmediateContext->PSSetShaderResources( 0u, 1u, pBackBufferSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO_GFX;
}

DxgiInfoQueue& Graphics::getInfoQueue()
{
#if defined _DEBUG && !defined NDEBUG
	return m_infoQueue;
#endif
}

KeyTimer<std::chrono::microseconds>& Graphics::getFpsTimer() noexcept
{
	return m_fpsTimer;
}

bool Graphics::getDisplayMode() const noexcept
{
	return m_bFullscreenMode;
}

bool& Graphics::getDisplayMode()
{
	return m_bFullscreenMode;
}

void Graphics::createFactory()
{
	HRESULT hres;
#ifdef FLIP_PRESENT
	hres = CreateDXGIFactory1( IID_PPV_ARGS( &m_pDxgiFactory ) );
//#if defined _DEBUG && !defined NDEBUG
//	unsigned dxgiFactoryFlags = 0u;
//	dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
//#endif
//	hres = CreateDXGIFactory2( dxgiFactoryFlags, IID_PPV_ARGS( &m_pDxgiFactory ) );
#else
	hres = CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), reinterpret_cast<void**>( m_pDxgiFactory.GetAddressOf() ) );
#endif
	ASSERT_HRES_IF_FAILED;

#if defined _DEBUG && !defined NDEBUG
	const char *dxgiFactoryDebugName = "KeyDxgiFactory";
	m_pDxgiFactory->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( dxgiFactoryDebugName ), dxgiFactoryDebugName );
#endif
}

void Graphics::createAdapters()
{
	unsigned adapterIndex = 0u;
	IDXGIAdapter *pAdapter = nullptr;	// IDXGIAdapter1 for blt, or IDXGIAdapter2 for flip model, or # for DXGI later versions

	while ( SUCCEEDED( m_pDxgiFactory->EnumAdapters( adapterIndex, &pAdapter ) ) )
	{
		s_adapters.emplace_back( pAdapter );
		adapterIndex += 1;
	}
}

//void Graphics::render3dSceneToBitmap()
//{
//	ASSERT( m_p2DSurface, "DXGISurface has not been created!" );
//	HRESULT hres = m_p2DRenderTarget->CreateSharedBitmap( );
//	ASSERT_HRES_IF_FAILED;
//
//	// you can render this bitmap with Direct2d
//}

void Graphics::recordDeferredCommandList()
{
	//HRESULT hres;
	//hres = m_pDevice->CreateDeferredContext( 0, &pDeferredContexts );
	//// Use the deferred context to render:
	//float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	//pDeferredContexts->ClearRenderTargetView( pRenderTargetView, ClearColor );
	//// Add additional rendering commands
	//// ...
	//hres = pDeferredContexts->FinishCommandList( FALSE, &pCommandList);
}

void Graphics::playbackDeferredCommandList()
{
	//if( pCommandList )
	//{
	//	m_pImmediateContext->ExecuteCommandList( pCommandList, TRUE );
	//}
}

void Graphics::interrogateDirectxFeatures()
{
	// check for DirectX Math library support
	if ( !dx::XMVerifyCPUSupport() )
	{
		MessageBoxW( m_hParentWnd, L"DirectX Math library support validation failure.", L"Error", MB_OK );
	}

	// threading
	D3D11_FEATURE_DATA_THREADING threadingInfo;
	ZeroMemory( &threadingInfo, sizeof( D3D11_FEATURE_DATA_THREADING ) );
	HRESULT hres = m_pDevice->CheckFeatureSupport( D3D11_FEATURE::D3D11_FEATURE_THREADING, &threadingInfo, sizeof( D3D11_FEATURE_DATA_THREADING ) );
	ASSERT_HRES_IF_FAILED;

	using namespace std::string_literals;
	auto &con = KeyConsole::getInstance();
	con.log( threadingInfo.DriverConcurrentCreates ?
		"Resources can be created concurrently on multiple threads.\n"s :
		"No DirectX concurrency possible\n"s );

	con.log( threadingInfo.DriverCommandLists ?
		"Command lists are supported by the current driver.\n"s :
		"Commands lists will be emulated in software.\n"s );

	D3D11_FEATURE_DATA_D3D11_OPTIONS d3d11HwOptions{};
	hres = m_pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D11_OPTIONS, &d3d11HwOptions, sizeof( d3d11HwOptions ) );
	ASSERT_HRES_IF_FAILED;

	if ( !d3d11HwOptions.MapNoOverwriteOnDynamicConstantBuffer )
	{
		con.log( "Constant Buffer D3D11_MAP_WRITE_NO_OVERWRITE unsupported!\n"s );
	}
	if ( !d3d11HwOptions.MapNoOverwriteOnDynamicBufferSRV )
	{
		con.log( "Shader Resource View D3D11_MAP_WRITE_NO_OVERWRITE unsupported!\n"s );
	}

	unsigned formatSupport;
	hres = m_pDevice->CheckFormatSupport( DXGI_FORMAT_B8G8R8A8_UNORM, &formatSupport );
	ASSERT_HRES_IF_FAILED;
}

bool Graphics::checkTearingSupport()
{
	bool bAllowTearing = false;
	// for Variable Refresh Rate displays (VRR) (VSync OFF)
	// Rather than create the 1.5 factory interface directly, we query for the 1.5 interface using a previous version factory.
	// This will enable the graphics debugging tools which might not support the 1.5 factory interface.
	mwrl::ComPtr<IDXGIFactory5> factory5;
	HRESULT hres = m_pDxgiFactory.As( &factory5 );
	ASSERT_HRES_IF_FAILED;

	hres = factory5->CheckFeatureSupport( DXGI_FEATURE_PRESENT_ALLOW_TEARING, &bAllowTearing, sizeof bAllowTearing );

	bAllowTearing = SUCCEEDED( hres ) && bAllowTearing;

	return bAllowTearing;
}

void Graphics::d3d11DebugReport()
{
#if defined _DEBUG && !defined NDEBUG
	HRESULT hres;
	hres = m_pDevice->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &m_pDebug ) );
	ASSERT_HRES_IF_FAILED;

	// report any live objects
	if ( m_pDebug != nullptr )
	{
		OutputDebugStringW( L"\n\nReporting Debug Objects:\n" );
		hres = m_pDebug->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
		ASSERT_HRES_IF_FAILED;
		OutputDebugStringW( L"\n\n" );
		m_pDebug = nullptr;
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef D2D_INTEROP
/// 2d & 3d Interoperability
void Graphics::create2dInteroperability()
{
	ASSERT( m_p2DSurface, "DXGISurface has not been created!" );

	D2D1_FACTORY_OPTIONS d2dOptions{};

#if defined _DEBUG && !defined NDEBUG
	d2dOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
	HRESULT hres = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof( ID2D1Factory1 ), &d2dOptions, &m_p2DFactory );
	ASSERT_HRES_IF_FAILED;

	RECT rect;
	GetClientRect( m_hParentWnd, &rect );
	auto size = D2D1::SizeU( rect.right - rect.left, rect.bottom - rect.top );
	hres = m_p2DFactory->CreateHwndRenderTarget( D2D1::RenderTargetProperties( D2D1_RENDER_TARGET_TYPE_HARDWARE, D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED ) ), D2D1::HwndRenderTargetProperties( m_hParentWnd, size ), &m_p2DRenderTarget );
	// or use CreateDxgiSurfaceRenderTarget to create a ID2D1RenderTarget (a subclass of ID2D1HwndRenderTarget) to draw your 2d content

	hres = m_p2DRenderTarget->QueryInterface( __uuidof( ID2D1DeviceContext ), reinterpret_cast<void**>( m_p2DContext.GetAddressOf() ) );
	ASSERT_HRES_IF_FAILED;

	ASSERT( m_p2DContext->IsDxgiFormatSupported( DXGI_FORMAT_B8G8R8A8_UNORM ) == TRUE, "DXGI_FORMAT_B8G8R8A8_UNORM is not supported on this Device Context!" );

	hres = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast<IUnknown**>( m_pWriteFactory.GetAddressOf() ) );
	ASSERT_HRES_IF_FAILED;
}

Microsoft::WRL::ComPtr<IDXGISurface>& Graphics::surface2d()
{
	return m_p2DSurface;
}

ID2D1HwndRenderTarget* Graphics::renderTarget2d()
{
	ASSERT( m_p2DSurface, "DXGISurface has not been created!" );
	return m_p2DRenderTarget.Get();
}

void Graphics::createTextFormat( const std::wstring &fontName,
	const float fontSize,
	const std::wstring &fontLocale )
{
	HRESULT hres = m_pWriteFactory->CreateTextFormat( fontName.c_str(), nullptr, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, fontLocale.c_str(), &m_pTextFormat );
	ASSERT_HRES_IF_FAILED;
}

void Graphics::drawLine( const D2D1_POINT_2F &v0,
	const D2D1_POINT_2F &v1,
	const D2D1::ColorF &rgba,
	const float strokeWidth )
{
	HRESULT hres = m_p2DRenderTarget->CreateSolidColorBrush( rgba, &m_p2dSolidColorBrush );
	ASSERT_HRES_IF_FAILED;
	m_p2DRenderTarget->DrawLine( v0, v1, m_p2dSolidColorBrush.Get(), strokeWidth );
}

void Graphics::drawRect( const D2D1_RECT_F &rect,
	const D2D1::ColorF &rgba,
	const float strokeWidth )
{
	HRESULT hres = m_p2DRenderTarget->CreateSolidColorBrush( rgba, &m_p2dSolidColorBrush );
	ASSERT_HRES_IF_FAILED;
	m_p2DRenderTarget->DrawRectangle( rect, m_p2dSolidColorBrush.Get(), strokeWidth );
}

void Graphics::drawRoundedRect( const D2D1_RECT_F &rect,
	const float radiusX,
	const float radiusY,
	const D2D1::ColorF &rgba,
	const float strokeWidth )
{
	HRESULT hres = m_p2DRenderTarget->CreateSolidColorBrush( rgba, &m_p2dSolidColorBrush );
	ASSERT_HRES_IF_FAILED;
	D2D1_ROUNDED_RECT roundRect{rect, radiusX, radiusY};
	m_p2DRenderTarget->DrawRoundedRectangle( &roundRect, m_p2dSolidColorBrush.Get(), strokeWidth );
}

void Graphics::drawEllipse( const float x,
	const float y,
	const float hRadius,
	const float vRadius,
	const D2D1::ColorF &rgba,
	const float strokeWidth )
{
	HRESULT hres = m_p2DRenderTarget->CreateSolidColorBrush( rgba, &m_p2dSolidColorBrush );
	ASSERT_HRES_IF_FAILED;
	m_p2DRenderTarget->DrawEllipse( D2D1::Ellipse( D2D1::Point2F(x, y), hRadius, vRadius ), m_p2dSolidColorBrush.Get(), strokeWidth );
}

void Graphics::drawCircle( const float x,
	const float y,
	const float radius,
	const D2D1::ColorF &rgba,
	const float strokeWidth )
{
	drawEllipse( x, y, radius, radius, rgba, strokeWidth );
}

void Graphics::drawText( const std::wstring &txt,
	const D2D1_RECT_F &rect,
	const D2D1::ColorF &rgba )
{
	HRESULT hres = m_p2DRenderTarget->CreateSolidColorBrush( rgba, &m_p2dSolidColorBrush );
	ASSERT_HRES_IF_FAILED;
	m_p2DRenderTarget->DrawTextW( txt.data(), txt.length(), m_pTextFormat.Get(), rect, m_p2dSolidColorBrush.Get() );
}
#endif	// D2D_INTEROP
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/// 2d only
ColorBGRA Graphics::getPixel( const int x,
	const int y ) const noexcept
{
	ASSERT( x >= 0, "!( x >= 0 )" );
	ASSERT( x < (int) m_width, "!( x < width )" );
	ASSERT( y >= 0, "!( y >= 0)" );
	ASSERT( y < (int) m_height, "!( y < height )" );
	return m_pCpuBuffer[m_width * y + x];
}

void Graphics::putPixel( const int x,
	const int y,
	const ColorBGRA color )
{
	ASSERT( x >= 0, "!( x >= 0 )" );
	ASSERT( x < (int) m_width, "!( x < width )" );
	ASSERT( y >= 0, "!( y >= 0)" );
	ASSERT( y < (int) m_height, "!( y < height )" );
	m_pCpuBuffer[m_width * y + x] = color;
}

void Graphics::drawLine( int x0,
	int x1,
	int y0,
	int y1,
	const ColorBGRA col )
{
	float m = 0.0f;
	if ( x1 != x0 )
	{
		m = (float) ( y1 - y0 ) / ( x1 - x0 );
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
			const int yi = (const int) y;
			if ( x >= 0 && x < (int) m_width && yi >= 0 && yi < (int) m_height )
			{
				putPixel( x, yi, col );
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

		const float w = (const float) ( x1 - x0 ) / ( y1 - y0 );
		const float p = x0 - w * y0;
		for ( int y = y0; y <= y1; ++y )
		{
			const float x = w * (float)y + p;
			const int xi = (const int) x;
			if ( xi >= 0 && xi < (int) m_width && y >= 0 && y < (int) m_height )
			{
				putPixel( xi, y, col );
			}
		}
	}
}

void Graphics::drawRectangle( int x0,
	int y0,
	int x1,
	int y1,
	const ColorBGRA col )
{
	if ( x0 > x1 )
	{
		std::swap( x0, x1 );
	}
	if ( y0 > y1 )
	{
		std::swap( y0, y1 );
	}

	for ( int x = x0; x < x1; ++x )
	{
		for ( int y = y0; y < y1; ++y )
		{
			putPixel( x, y, col );
		}
	}
}

void Graphics::drawTriangle( int x0,
	int y0,
	int x1,
	int y1,
	int x2,
	int y2,
	const ColorBGRA col )
{
	// line for {x0, y0}, {x1, y1}
	drawLine(x0, x1, y0, y1, col);
	// line for {x0, y0}, {x2, y2}
	drawLine(x0, x2, y0, y2, col);
	// line for {x1, y1}, {x2, y2}
	drawLine(x1, x2, y1, y2, col);
}

void Graphics::drawCircle( const int centerX,
	const int centerY,
	const int radius,
	const ColorBGRA col )
{
	const int radiusSquared = radius * radius;
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
				putPixel( xLoop, yLoop, col );
			}
		}
	}
}

void Graphics::drawStar( const float outerRadius,
	const float innerRadius,
	const ColorBGRA color,
	const int nFlares /*= 5 */)
{
	std::vector<dx::XMFLOAT2> star;
	star.reserve( (size_t)nFlares * 2 );
	const float dTheta = 2.0f * 3.14159f / float( nFlares * 2 );
	for ( int i = 0; i < nFlares * 2; ++i )
	{
		const float rad = ( i % 2 == 0 ) ?
			outerRadius :
			innerRadius;
		star.emplace_back( rad * cos( float(i) * dTheta ), rad * sin( float(i) * dTheta ) );
	}

	for ( const dx::XMFLOAT2 &coord : star )
	{
		putPixel( coord.x, coord.y, color );
	}
}

//drawBezier
// general implementation for a curve with any number of points.
/*
putPixel( getBezierPoint() )
vec2 getBezierPoint( vec2 *points, int numPoints, float t )
{
	vec2 *tmp = new vec2[numPoints];
	memcpy(tmp, points, numPoints * sizeof(vec2));
	int i = numPoints - 1;
	while (i > 0)
	{
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

private Vector3f getPointOnRay(Vector3f ray, float distance)
{
	Vector3f camPos = camera.getPosition();
	Vector3f start = new Vector3f(camPos.x, camPos.y, camPos.z);
	Vector3f scaledRay = new Vector3f(ray.x * distance, ray.y * distance, ray.z * distance);
	return Vector3f.add(start, scaledRay, null);
}

private Vector3f binarySearch(int count, float start, float finish, Vector3f ray) {
	float half = start + ((finish - start) / 2f);
	if (count >= RECURSION_COUNT)
	{
		Vector3f endPoint = getPointOnRay(ray, half);
		Terrain terrain = getTerrain(endPoint.getX(), endPoint.getZ());
		if (terrain != null)
		{
			return endPoint;
		}
		else
		{
			return null;
		}
	}
	if (intersectionInRange(start, half, ray))
	{
		return binarySearch(count + 1, start, half, ray);
	}
	else
	{
		return binarySearch(count + 1, half, finish, ray);
	}
}

private boolean intersectionInRange(float start, float finish, Vector3f ray) {
	Vector3f startPoint = getPointOnRay(ray, start);
	Vector3f endPoint = getPointOnRay(ray, finish);
	if (!isUnderGround(startPoint) && isUnderGround(endPoint))
	{
		return true;
	}
	else
	{
		return false;
	}
}

private boolean isUnderGround(Vector3f testPoint) {
	Terrain terrain = getTerrain(testPoint.getX(), testPoint.getZ());
	float height = 0;
	if (terrain != null)
	{
		height = terrain.getHeightOfTerrain(testPoint.getX(), testPoint.getZ());
	}
	if (testPoint.y < height)
	{
		return true;
	}
	else
	{
		return false;
	}
}*/

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
Graphics::GraphicsException::GraphicsException( const int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	KeyException{line, file, function, msg}
{

}

std::string Graphics::GraphicsException::getType() const noexcept
{
	return typeid( *this ).name();
}

const char* Graphics::GraphicsException::what() const noexcept
{
	return KeyException::what();
}