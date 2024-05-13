#pragma once

#include "winner.h"
#include "key_wrl.h"
#include "dxgi1_4.h"
#include <d3d11.h>
#ifdef D2D_ONLY
#	include <d2d1_1.h>
#	include <dwrite.h>
#endif
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include <random>
#include <vector>
#if defined _DEBUG && !defined NDEBUG
#	include <atlbase.h>
#endif
#include "non_copyable.h"
#include "key_exception.h"
#if defined _DEBUG && !defined NDEBUG
#	include "dxgi_info_queue.h"
#endif
#include "color.h"
#include "texture_desc.h"
#include "key_timer.h"


class IBindable;
class RenderTargetOutput;
class DepthStencilOutput;
class TextureOffscreenRT;
class TextureOffscreenDS;
class Window;
class Camera;
struct R3ctangle;

namespace ren
{

class Renderer;
class Renderer3d;
class Renderer2d;

}

//=============================================================
//	\class	Graphics
//	\author	KeyC0de
//	\date	2022/09/13 22:51
//	\brief	Graphics API (d3d11) encapsulator
//			provides classic blt presentation functionality
//			as well as upgraded DXGI 1.2 API with Flip enhanced presentation model (aiming for Independent Flip)
//=============================================================
class Graphics final
	: NonCopyableAndNonMovable
{
	friend class GraphicsFriend;

	class GraphicsException final
		: public KeyException
	{
	public:
		GraphicsException( const int line, const char *file, const char *function, const std::string &msg ) noexcept;

		std::string getType() const noexcept override final;
		virtual const char* what() const noexcept override final;
	};

	class Adapter final
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> m_pAdapter;
		DXGI_ADAPTER_DESC m_desc;
	public:
		Adapter( IDXGIAdapter *pAdapter );
		const DXGI_ADAPTER_DESC* getDesc() const noexcept;
		IDXGIAdapter* getAdapter() const noexcept;
		void getVRamDetails() const noexcept;
	};
private:
	static inline D3D_FEATURE_LEVEL s_featureLevel;
	static inline std::vector<Adapter> s_adapters;
private:
	unsigned m_width;
	unsigned m_height;
	HWND m_hParentWnd;
	UINT m_swapChainFlags = 0u;
#if defined FLIP_PRESENT
	unsigned m_presentFlags = 0u;	// only valid in windowed mode
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_pDxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_pSwapChain;
#else
	Microsoft::WRL::ComPtr<IDXGIFactory1> m_pDxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain;
#endif
	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pImmediateContext;
	Microsoft::WRL::ComPtr<IDXGIOutput1> m_pDxgiOutput;
	std::shared_ptr<RenderTargetOutput> m_pBackBufferRtv;
	std::shared_ptr<DepthStencilOutput> m_pBackBufferDsv;
	std::shared_ptr<TextureOffscreenRT> m_pOffscreenRtv;
	std::shared_ptr<TextureOffscreenDS> m_pOffscreenDsv;
	DxgiInfoQueue m_infoQueue;
	ATL::CComPtr<ID3D11Debug> m_pDebug;
	DirectX::XMMATRIX m_projection;
	DirectX::XMMATRIX m_view;
	size_t m_currentFrame = 0u;
	std::vector<ID3D11DeviceContext*> m_deferredContexts;
	std::vector<ID3D11CommandList*> m_commandLists;
	std::unique_ptr<ren::Renderer> m_pRenderer;
	ren::Renderer3d* m_pRenderer3d;
	ren::Renderer2d* m_pRenderer2d;
	KeyTimer<std::chrono::microseconds> m_fpsTimer;
	bool m_bFullscreenMode = false;
public:
	Graphics( const HWND hWnd, const int width, const int height, const MultisamplingMode multisamplingMode = MultisamplingMode::None );
	~Graphics();

	void makeWindowAssociationWithFactory( HWND hWnd, const UINT flags );
	void profile() const noexcept;
	void clearShaderSlots() noexcept;
	void cleanState() noexcept;
	void beginFrame() noexcept;
	void endFrame();
	void draw( const unsigned count ) cond_noex;
	void drawIndexed( const unsigned count ) cond_noex;
	void drawIndexedInstanced( const unsigned indexCount, const unsigned instanceCount ) cond_noex;
	ColorBGRA*& cpuBuffer();
	void setViewMatrix( const DirectX::XMMATRIX &cam ) noexcept;
	void setProjectionMatrix( const DirectX::XMMATRIX &proj ) noexcept;
	const DirectX::XMMATRIX& getViewMatrix() const noexcept;
	const DirectX::XMMATRIX& getProjectionMatrix() const noexcept;
	unsigned getClientWidth() const noexcept;
	unsigned getClientHeight() const noexcept;
	std::shared_ptr<RenderTargetOutput> getRenderTargetFromBackBuffer();
	std::shared_ptr<DepthStencilOutput> getDepthBufferFromBackBuffer();
	std::shared_ptr<TextureOffscreenRT> getRenderTargetOffscreen( const unsigned slot, const RenderTargetViewMode rtvMode );
	std::shared_ptr<TextureOffscreenDS> getDepthBufferOffscreen( const unsigned slot, const DepthStencilViewMode dsvMode );
	void bindBackBufferAsInput();
	void createFactory();
	void createAdapters();
	//	\function	resize	||	\date	2022/09/17 19:44
	//	\brief	sets windowed mode or Fullscreen, supply width & height of 0 to resize the buffers for fullscreen mode usage
	void resize( unsigned newWidth, unsigned newHeight );
	//	\function	setupMonitors	||	\date	2022/09/18 19:08
	//	\brief	sets up the output monitor devices
	void setupMonitors();
	double getRefreshRate() const noexcept;
	size_t getFrameNum() const noexcept;
	void runRenderer() noexcept;
	ren::Renderer& getRenderer() noexcept;
	ren::Renderer3d& getRenderer3d() noexcept;
	ren::Renderer2d& getRenderer2d() noexcept;
	DxgiInfoQueue& getInfoQueue();
	KeyTimer<std::chrono::microseconds>& getFpsTimer() noexcept;
	//	\function	getDisplayMode	||	\date	2024/05/03 18:18
	//	\brief	returns true if fullscreen application, false otherwise
	bool getDisplayMode() const noexcept;
	bool& getDisplayMode();
	//	\function	recordDeferredCommandList	||	\date	2022/08/21 14:05
	//	\brief	probably should call this when the Model is being loaded not when pass->run() -> Job->run()
	void recordDeferredCommandList();
	//	\function	playbackDeferredCommandList	||	\date	2020/11/05 14:51
	//	\brief		ExecuteCommandList must be executed on the immediate context for recorded commands to be run on the GPU
	void playbackDeferredCommandList();
private:
	//	\function	present	||	\date	2022/09/13 22:12
	//	\brief	present the frame to DWM
	void present();
	void interrogateDirectxFeatures();
	bool checkTearingSupport();
	void d3d11DebugReport();
private:
	/// d2d via d3d Interoperability
	ColorBGRA *m_pCpuBuffer = nullptr;
public:
	ColorBGRA getPixel( const int x, const int y ) const noexcept;
	void putPixel( const int x, const int y, const ColorBGRA color );
	void putPixel( const int x, const int y, const int r, const int g, const int b );
	//	\function	drawLine	||	\date	2021/04/26 21:27
	//	\brief	uses the Bresenham algorithm to draw lines, ie. connect 2d positions together by drawing straight lines between them
	//			this is one of the algorithms by D3D as well during rasterization
	void drawLine( int x0, int x1, int y0, int y1, const ColorBGRA col );
	void drawRectangle( int x0, int y0, int x1, int y1, const ColorBGRA col );
	void drawRectangle( const R3ctangle &rect, const ColorBGRA col );
	void drawRectWH( const int x0, const int y0, const int width, const int height, const ColorBGRA col );
	void drawTriangle( int x0, int y0, int x1, int y1, int x2, int y2, const ColorBGRA col );
	void drawCircle( const int centerX, const int centerY, const int radius, const ColorBGRA col );
	void drawStar( const float outerRadius, const float innerRadius, const ColorBGRA color, const int nFlares = 5);
#ifdef D2D_ONLY
private:
	/// 2d only
	Microsoft::WRL::ComPtr<ID2D1Factory> m_p2DFactory;
	Microsoft::WRL::ComPtr<IDXGISurface> m_p2DSurface;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_p2DRenderTarget;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_p2DContext;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_p2dSolidColorBrush;
	Microsoft::WRL::ComPtr<IDWriteFactory> m_pWriteFactory;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> m_pTextFormat;
private:
	void begin2dDraw();
	void end2dDraw();
	void clear( const D2D1::ColorF &rgb = D2D1::ColorF{0.0f, 0.0f, 0.0f} );
public:
	void create2dFactory();
	Microsoft::WRL::ComPtr<IDXGISurface>& surface2d();
	ID2D1HwndRenderTarget* renderTarget2d();

	void createTextFormat( const std::wstring& fontName = L"Arial", const float fontSize = 24.0f, const std::wstring &fontLocale = L"en-us" );
	void drawLine( const D2D1_POINT_2F &v0, const D2D1_POINT_2F &v1, const D2D1::ColorF &rgba, const float strokeWidth = 1.0f );
	void drawRect( const D2D1_RECT_F &rect, const D2D1::ColorF &rgba, const float strokeWidth = 1.0f );
	void drawRoundedRect( const D2D1_RECT_F &rect, const float radiusX, const float radiusY, const D2D1::ColorF &rgba, const float strokeWidth = 1.0f );
	void drawEllipse( const float x, const float y, const float hRadius, const float vRadius, const D2D1::ColorF &rgba, const float strokeWidth = 1.0f );
	void drawCircle( const float x, const float y, const float radius, const D2D1::ColorF &rgba, const float strokeWidth = 1.0f );
	void drawText( const std::wstring &txt, const D2D1_RECT_F &rect, const D2D1::ColorF &rgba );
#endif
};


#define THROW_GRAPHICS_EXCEPTION( msg ) __debugbreak();\
	throw GraphicsException( __LINE__, __FILE__, __FUNCTION__, msg );