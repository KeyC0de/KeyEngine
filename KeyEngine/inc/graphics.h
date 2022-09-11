#pragma once

#include "winner.h"
#include "key_wrl.h"
#include <d3d11.h>
#include <d2d1_1.h>
#include <dwrite.h>
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
#include "dxgi_info_queue.h"
#include "key_timer.h"
#include "SpriteFont.h"
#include "SpriteBatch.h"
#include "color.h"
#include "rectangle.h"


class IBindable;
class IRenderTargetView;
class Window;
class Camera;

class Graphics
	: NonCopyableAndNonMovable
{
	friend class GraphicsFriend;

	class GraphicsException final
		: public KeyException
	{
	public:
		GraphicsException( const int line, const char *file, const char *function, const std::string &msg ) noexcept;

		const std::string getType() const noexcept override final;
		virtual const char* what() const noexcept override final;
	};

	class Adapter final
	{
		DXGI_ADAPTER_DESC m_desc;
		Microsoft::WRL::ComPtr<IDXGIAdapter> m_pAdapter;
	public:
		Adapter( IDXGIAdapter *pAdapter );

		const DXGI_ADAPTER_DESC* getDesc() const noexcept;
		IDXGIAdapter* adapter() const noexcept;
	};

private:
	static inline D3D_FEATURE_LEVEL m_featureLevel;
	unsigned m_width;
	unsigned m_height;
	HWND m_hParentWnd;
	static inline std::vector<Adapter> m_adapters;
	Microsoft::WRL::ComPtr<IDXGIFactory1> m_pDxgiFactory;
	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
#if defined _FLIP_PRESENT
	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_pSwapChain;
#else
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain;
#endif
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pImmediateContext;
	std::shared_ptr<IRenderTargetView> m_globalColorBuffer;
#if defined _DEBUG && !defined NDEBUG
	DxgiInfoQueue m_infoQueue;
	ATL::CComPtr<ID3D11Debug> m_pDebug;
#endif
	KeyTimer<std::chrono::microseconds> m_fpsTimer;
	DirectX::XMMATRIX m_projection;
	DirectX::XMMATRIX m_view;
	std::vector<ID3D11DeviceContext*> m_deferredContexts;
	std::vector<ID3D11CommandList*> m_commandLists;
#if defined _FLIP_PRESENT
private:
	static bool checkTearingSupport();
#endif
public:
	Graphics( const HWND hWnd, const int width, const int height );
	~Graphics();

#if defined _FLIP_PRESENT
	void makeWindowAssociationWithFactory( HWND hWnd, const UINT flags = DXGI_MWA_NO_WINDOW_CHANGES );
#endif
	void beginFrame() noexcept;
	void updateAndRenderFpsTimer();
	void endFrame();
	void present();
	void draw( const unsigned count ) cond_noex;
	void drawIndexed( const unsigned count ) cond_noex;
	void drawIndexedInstanced( const unsigned indexCount, const unsigned instanceCount ) cond_noex;
	ColorBGRA*& cpuBuffer();
	void setViewMatrix( const DirectX::XMMATRIX &cam ) noexcept;
	void setProjectionMatrix( const DirectX::XMMATRIX &proj ) noexcept;
	const DirectX::XMMATRIX& getViewMatrix() const noexcept;
	const DirectX::XMMATRIX& getProjectionMatrix() const noexcept;
	const unsigned getClientWidth() const noexcept;
	const unsigned getClientHeight() const noexcept;
	std::shared_ptr<IRenderTargetView> shareRenderTarget();
	void createAdapters();

#if defined _DEBUG && !defined NDEBUG
	DxgiInfoQueue& infoQueue();
#endif
private:
	//===================================================
	//	\function	recordDeferredCommandList
	//	\brief  probably should call this when the Model is being loaded not when pass->run() -> Job->run()
	//	\date	2022/08/21 14:05
	void recordDeferredCommandList();
	//===================================================
	//	\function	playbackDeferredCommandList
	//	\brief  ExecuteCommandList must be executed on the immediate context for recorded
	//			commands to be run on the GPU
	//	\date	2020/11/05 14:51
	void playbackDeferredCommandList();
	void clearShaderSlots() noexcept;
	void cleanState() noexcept;
#if defined _DEBUG && !defined NDEBUG
	void interrogateDirectxFeatures();
	void d3d11DebugReport();
#endif
private:
	/// 2d & 3d Interoperability
	Microsoft::WRL::ComPtr<ID2D1Factory> m_p2DFactory;
	Microsoft::WRL::ComPtr<IDXGISurface> m_p2DSurface;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_p2DRenderTarget;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_p2DContext;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_p2dSolidColorBrush;
	Microsoft::WRL::ComPtr<IDWriteFactory> m_pWriteFactory;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> m_pTextFormat;
	std::unique_ptr<DirectX::SpriteFont> m_pSpriteFont;
	std::unique_ptr<DirectX::SpriteBatch> m_pFpsSpriteBatch;
private:
	inline void begin2dDraw()
	{
		m_p2DRenderTarget->BeginDraw();
	}
	inline void end2dDraw()
	{
		m_p2DRenderTarget->EndDraw();
	}
	inline void clear( const D2D1::ColorF &rgb = D2D1::ColorF{0.0f, 0.0f, 0.0f} )
	{
		m_p2DRenderTarget->Clear( rgb );
	}
public:
	void create2dInteroperability();
	Microsoft::WRL::ComPtr<IDXGISurface>& surface2d();
	ID2D1HwndRenderTarget* renderTarget2d();

	void createTextFormat( const std::wstring& fontName = L"Arial", const float fontSize = 24.0f, const std::wstring &fontLocale = L"en-us" );
	void drawLine( const D2D1_POINT_2F &v0, const D2D1_POINT_2F &v1, const D2D1::ColorF &rgba, const float strokeWidth = 1.0f );
	void drawRect( const D2D1_RECT_F &rect, const D2D1::ColorF &rgba, const float strokeWidth = 1.0f );
	void drawRoundedRect( const D2D1_RECT_F &rect, const float radiusX, const float radiusY, const D2D1::ColorF &rgba, const float strokeWidth = 1.0f );
	void drawEllipse( const float x, const float y, const float hRadius, const float vRadius, const D2D1::ColorF &rgba, const float strokeWidth = 1.0f );
	void drawCircle( const float x, const float y, const float radius, const D2D1::ColorF &rgba, const float strokeWidth = 1.0f );
	void drawText( const std::wstring &txt, const D2D1_RECT_F &rect, const D2D1::ColorF &rgba );

private:
	/// 2d only
	ColorBGRA *m_pCpuBuffer = nullptr;
public:
	const ColorBGRA getPixel( const int x, const int y ) const noexcept;
	void putPixel( const int x, const int y, const ColorBGRA col );
	inline void putPixel( const int x,
		const int y,
		const int r,
		const int g,
		const int b )
	{
		putPixel( x,
			y,
			{static_cast<BYTE>( r ), static_cast<BYTE>( g ), static_cast<BYTE>( b )} );
	}
	void drawLine( int x0, int x1, int y0, int y1, const ColorBGRA col );
	void drawRectangle( int x0, int y0, int x1, int y1, const ColorBGRA col );
	inline void drawRectangle( const Rect &rect, const ColorBGRA col )
	{
		drawRectangle( static_cast<int>( rect.m_left ),
			static_cast<int>( rect.m_top ),
			static_cast<int>( rect.m_right ),
			static_cast<int>( rect.m_bottom ),
			col );
	}

	inline void drawRectWH( const int x0,
		const int y0,
		const int width,
		const int height,
		const ColorBGRA col )
	{
		drawRectangle( x0,
			y0,
			x0 + width,
			y0 + height,
			col );
	}

	void drawTriangle( int x0, int y0, int x1, int y1, int x2, int y2, const ColorBGRA col );
	void drawCircle( const int centerX, const int centerY, const int radius, const ColorBGRA col );
	const std::vector<DirectX::XMFLOAT2> drawStar( const float outerRadius, const float innerRadius, const int nFlares = 5 );
};

#define THROW_GRAPHICS_EXCEPTION( msg ) throw GraphicsException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );\
	__debugbreak();