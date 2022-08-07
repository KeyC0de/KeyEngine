#pragma once

#include "winner.h"
#include "key_wrl.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include <random>
#include <vector>
#if defined _DEBUG && !defined NDEBUG
#	include <atlbase.h>
#endif
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
{
	friend class GraphicsFriend;

	class GraphicsException final
		: public KeyException
	{
	public:
		GraphicsException( int line, const char *file, const char *function,
			const std::string &msg ) noexcept;

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
		IDXGIAdapter* getAdapter() const noexcept;
	};

private:
	static inline std::vector<Adapter> m_adapters;
	static inline D3D_FEATURE_LEVEL m_featureLevel;
	unsigned m_width;
	unsigned m_height;
	DirectX::XMMATRIX m_projection;
	DirectX::XMMATRIX m_view;
#if defined _DEBUG && !defined NDEBUG
	std::unique_ptr<DxgiInfoQueue> m_infoQueue;
	ATL::CComPtr<ID3D11Debug> m_pDebug;	// try using PIMPL and declare it in .cpp
#endif
	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
#if defined _FLIP_PRESENT
	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_pSwapChain;
#else
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain;
#endif
	Microsoft::WRL::ComPtr<IDXGIFactory1> m_pFactory;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;	// immediate context
	std::shared_ptr<IRenderTargetView> m_globalColorBuffer;
	KeyTimer<std::chrono::microseconds> m_fpsTimer;
	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
	std::unique_ptr<DirectX::SpriteBatch> m_fpsSpriteBatch;
	std::vector<std::unique_ptr<ID3D11DeviceContext>> m_deferredContexts;
	std::vector<std::unique_ptr<ID3D11CommandList>> m_commandLists;
	ColorBGRA *m_pCpuBuffer = nullptr;
#if defined _FLIP_PRESENT
private:
	static bool checkTearingSupport();
#endif
public:
	Graphics( HWND hWnd, int width, int height );
	~Graphics();
	Graphics( const Graphics &rhs ) = delete;
	Graphics& operator=( const Graphics &rhs ) = delete;
	Graphics( Graphics &&rhs ) = delete;
	Graphics& operator=( Graphics &&rhs ) = delete;

#if defined _FLIP_PRESENT
	void makeWindowAssociationWithFactory( HWND hWnd, UINT flags = DXGI_MWA_NO_WINDOW_CHANGES );
#endif
	void beginRendering() noexcept;
	void updateAndRenderFpsTimer();
	void endRendering();
	void draw( unsigned count ) cond_noex;
	void drawIndexed( unsigned count ) cond_noex;
	void drawIndexedInstanced( unsigned indexCount, unsigned instanceCount ) cond_noex;
	ColorBGRA*& getCpuBuffer();
	void setViewMatrix( const DirectX::XMMATRIX &cam ) noexcept;
	void setProjectionMatrix( const DirectX::XMMATRIX &proj ) noexcept;
	DirectX::XMMATRIX getViewMatrix() const noexcept;
	DirectX::XMMATRIX getProjectionMatrix() const noexcept;
	unsigned getClientWidth() const noexcept;
	unsigned getClientHeight() const noexcept;
	IRenderTargetView* getRenderTarget() const noexcept;
	std::shared_ptr<IRenderTargetView> shareRenderTarget();
#if defined _DEBUG && !defined NDEBUG
	DxgiInfoQueue& getInfoQueue() const noexcept;
#endif
	void createAdapters();
	std::vector<Adapter>& getAdapters() const;

	// 2d
	ColorBGRA getPixel( int x, int y ) const noexcept;
	void putPixel( int x, int y, ColorBGRA col );
	inline void putPixel( int x,
		int y,
		int r,
		int g,
		int b )
	{
		putPixel( x,
			y,
			{static_cast<BYTE>( r ), static_cast<BYTE>( g ), static_cast<BYTE>( b )} );
	}
	void drawLine( int x0, int x1, int y0, int y1, ColorBGRA col );
	void drawRect( int x0, int y0, int x1, int y1, ColorBGRA col );
	inline void drawRect( const Rect &rect,
		ColorBGRA col )
	{
		drawRect( static_cast<int>( rect.m_left ),
			static_cast<int>( rect.m_top ),
			static_cast<int>( rect.m_right ),
			static_cast<int>( rect.m_bottom ),
			col );
	}

	inline void drawRectWH( int x0,
		int y0,
		int width,
		int height,
		ColorBGRA col )
	{
		drawRect( x0,
			y0,
			x0 + width,
			y0 + height,
			col );
	}

	void drawTriangle();
	void drawCircle( int centerX, int centerY, int radius, ColorBGRA col );
	std::vector<DirectX::XMFLOAT2> drawStar( float outerRadius, float innerRadius, int nFlares = 5 );
private:
	void recordDeferredCommandList();
	//===================================================
	//	\function	playbackDeferredCommandList
	//	\brief  ExecuteCommandList must be executed on the immediate context for recorded
	//			commands to be run on the GPU
	//	\date	2020/11/05 14:51
	void playbackDeferredCommandList();
	void clearShaderSlots() noexcept;
#if defined _DEBUG && !defined NDEBUG
	void interrogateDirectxFeatures();
	void d3d11DebugReport();
#endif
};

#define THROW_GRAPHICS_EXCEPTION( msg ) throw GraphicsException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );