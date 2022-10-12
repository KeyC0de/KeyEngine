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
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRtv;
protected:
	//	\function	IrenderTargetView	||	\date	2021/10/25 17:00
	//	\brief  Render to Texture constructor
	IRenderTargetView( Graphics &gph, const unsigned width, const unsigned height );
	//	\function	IrenderTargetView	||	\date	2021/10/25 17:00
	//	\brief  For creating an output RT only
	IRenderTargetView( Graphics &gph, ID3D11Texture2D *pTex, std::optional<unsigned> face );
public:
	void bindRenderSurface( Graphics &gph ) cond_noex override;
	void bindRenderSurface( Graphics &gph, IRenderSurface *pRs ) cond_noex override;
	void bindRenderSurface( Graphics &gph, IDepthStencilView *pDepthStencilView ) cond_noex;
	void clear( Graphics &gph, const std::array<float, 4> &color = {0.0f, 0.0f, 0.0f, 0.0f} ) cond_noex override;
	void clean( Graphics &gph ) cond_noex;
	const unsigned getWidth() const noexcept;
	const unsigned getHeight() const noexcept;
	ID3D11RenderTargetView* renderTargetView() const noexcept;
	const Bitmap convertToBitmap( Graphics &gph, const unsigned width, const unsigned height ) const;
private:
	//	\function	createStagingTexture	||	\date	2021/10/27 21:59
	//	\brief  create a texture resource compatible with our rtv, but with Staging usage (CPU read access, no GPU access)
	std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC> createStagingTexture( Graphics &gph ) const;
	void bindRenderSurface( Graphics &gph, ID3D11DepthStencilView *pD3dDsv ) cond_noex;
};

class RenderTargetShaderInput
	: public IRenderTargetView
{
	unsigned m_slot;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSrv;
public:
	RenderTargetShaderInput( Graphics &gph, const unsigned width, const unsigned height, const unsigned slot );

	void bind( Graphics &gph ) cond_noex override;
};

//=============================================================
//	\class	RenderTargetOutput
//	\author	KeyC0de
//	\date	2021/10/19 0:47
//	\brief	render target view used for Output only (offscreen/RTT/back buffer rendering)
//=============================================================
class RenderTargetOutput
	: public IRenderTargetView
{
public:
	RenderTargetOutput( Graphics &gph, ID3D11Texture2D *pTex, std::optional<unsigned> face = {} );
	RenderTargetOutput( Graphics &gph, const unsigned width, const unsigned height, const unsigned slot );	// no users

	void bind( Graphics &gph ) cond_noex override;
};