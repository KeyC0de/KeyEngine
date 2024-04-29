#pragma once

#include <optional>
#include "bindable.h"
#include "render_surface.h"


class Graphics;
class Bitmap;
class IDepthStencilView;

class IRenderTargetView
	: public IBindable,
	public IRenderSurface
{
protected:
	unsigned m_width;
	unsigned m_height;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pD3dRtv;
protected:
	//	\function	IrenderTargetView	||	\date	2021/10/25 17:00
	//	\brief  For creating an output RT only
	IRenderTargetView( Graphics &gph, ID3D11Texture2D *pTex, std::optional<unsigned> face );
	//	\function	IrenderTargetView	||	\date	2021/10/25 17:00
	//	\brief  Render to Texture constructor
	IRenderTargetView( Graphics &gph, const unsigned width, const unsigned height );
public:
	//	\function	unbind	||	\date	2022/10/29 21:56
	//	\brief	unbind targets from Output - currently UNUSED
	static void unbind( Graphics &gph ) noexcept;
public:
	void bindRenderSurface( Graphics &gph ) cond_noex override;
	void bindRenderSurface( Graphics &gph, IRenderSurface *pRs ) cond_noex override;
	void bindRenderSurface( Graphics &gph, IDepthStencilView *pDepthStencilView ) cond_noex;
	void clear( Graphics &gph, const std::array<float, 4> &color = {0.0f, 0.0f, 0.0f, 0.0f} ) cond_noex override;
	void clean( Graphics &gph ) cond_noex;
	const Bitmap convertToBitmap( Graphics &gph, const unsigned width, const unsigned height ) const;
	unsigned getWidth() const noexcept;
	unsigned getHeight() const noexcept;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& d3dResourceCom() noexcept;
	ID3D11RenderTargetView* d3dResource() const noexcept;
	virtual void setDebugObjectName( const char* name ) noexcept override;
private:
	//	\function	createStagingTexture	||	\date	2021/10/27 21:59
	//	\brief  create a texture resource compatible with our RTV, but with Staging usage (CPU read access, no GPU access)
	std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC> createStagingTexture( Graphics &gph ) const;
	void bindRenderSurface( Graphics &gph, ID3D11DepthStencilView *pD3dDsv ) cond_noex;
};

//=============================================================
//	\class	RenderTargetShaderInput
//	\author	KeyC0de
//	\date	2022/10/13 11:09
//	\brief	RTV wrapper that will be used for Input from the Back Buffer only
//=============================================================
class RenderTargetShaderInput
	: public IRenderTargetView
{
	unsigned m_slot;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pD3dSrv;
public:
	RenderTargetShaderInput( Graphics &gph, const unsigned width, const unsigned height, const unsigned slot, const RenderTargetViewMode rtvMode = DefaultRT );

	void bind( Graphics &gph ) cond_noex override;
	unsigned getSlot() const noexcept;
};

//=============================================================
//	\class	RenderTargetOutput
//	\author	KeyC0de
//	\date	2021/10/19 0:47
//	\brief	RTV wrapper that will be used for Output only
//=============================================================
class RenderTargetOutput
	: public IRenderTargetView
{
public:
	RenderTargetOutput( Graphics &gph, ID3D11Texture2D *pTex, std::optional<unsigned> face = {} );

	void bind( Graphics &gph ) cond_noex override;
};