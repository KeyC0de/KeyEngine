#pragma once

#include "bindable.h"
#include "render_surface.h"
#include "bitmap.h"


class Graphics;
class IRenderTargetView;

class IDepthStencilView
	: public IBindable,
	public IRenderSurface
{
	friend IRenderTargetView;

	std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC> createStagingTexture( Graphics &gph ) const;
protected:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pD3dDsv;
	unsigned int m_width;
	unsigned int m_height;
protected:
	IDepthStencilView( Graphics &gph, ID3D11Texture2D *pTexture, const DepthStencilViewMode dsMode, std::optional<unsigned> face );
	IDepthStencilView( Graphics &gph, const unsigned width, const unsigned height, const bool bBindAsShaderInput, const DepthStencilViewMode dsMode );
public:
	void bindRenderSurface( Graphics &gph ) cond_noex override;
	void bindRenderSurface( Graphics &gph, IRenderSurface *rt ) cond_noex override;
	void bindRenderSurface( Graphics &gph, IRenderTargetView *rt ) cond_noex;
	void clear( Graphics &gph, const std::array<float, 4> &unused = {} ) cond_noex override;
	void clean( Graphics &gph ) cond_noex;
	//	\function	convertToBitmap	||	\date	2021/06/25 18:55
	//	\brief  copy from resource to staging --- linearize & normalize depth values by default
	const Bitmap convertToBitmap( Graphics &gph, const unsigned width, const unsigned height, bool bLinearize = true ) const;
	const unsigned int getWidth() const noexcept;
	const unsigned int getHeight() const noexcept;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& d3dResourceCom() noexcept;
	ID3D11DepthStencilView* d3dResource() const noexcept;
	virtual void setDebugObjectName( const char* name ) noexcept override;
};

class DepthStencilShaderInput
	: public IDepthStencilView
{
	unsigned int m_slot;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pD3dSrv;
public:
	DepthStencilShaderInput( Graphics &gph, const unsigned slot, const DepthStencilViewMode dsMode = DepthStencilViewMode::Normal );
	DepthStencilShaderInput( Graphics &gph, const unsigned width, const unsigned height, const unsigned slot, const DepthStencilViewMode dsMode = DepthStencilViewMode::Normal );

	void bind( Graphics &gph ) cond_noex override;
	unsigned getSlot() const noexcept;
};

//=============================================================
//	\class	DepthStencilOutput
//	\author	KeyC0de
//	\date	2021/10/19 0:47
//	\brief	depth stencil view used for Output only (offscreen/RTT/back buffer rendering)
//=============================================================
class DepthStencilOutput
	: public IDepthStencilView
{
public:
	DepthStencilOutput( Graphics &gph );
	DepthStencilOutput( Graphics &gph, const unsigned width, const unsigned height );
	DepthStencilOutput( Graphics &gph, ID3D11Texture2D *pTexture, const DepthStencilViewMode dsMode, std::optional<unsigned> face = {} );

	void bind( Graphics &gph ) cond_noex override;
};