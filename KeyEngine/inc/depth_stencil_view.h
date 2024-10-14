#pragma once

#include "key_wrl.h"
#include <optional>
#include "bindable.h"
#include "render_surface.h"
#include "bitmap.h"
#include "texture_desc.h"


class Graphics;
class IRenderTargetView;

class IDepthStencilView
	: public IBindable,
	public IRenderSurface
{
	friend IRenderTargetView;

protected:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pD3dDsv;
	unsigned int m_width;
	unsigned int m_height;
protected:
	//	\function	IDepthStencilView	||	\date	2021/10/25 17:00
	//	\brief	For creating an output DS only
	IDepthStencilView( Graphics &gfx, ID3D11Texture2D *pTexture, const DepthStencilViewMode dsvMode, std::optional<unsigned> face );
	//	\function	IDepthStencilView	||	\date	2024/09/03 20:31
	//	\brief	for creating an output DS only but with Texture*Array semantics
	IDepthStencilView( Graphics &gfx, ID3D11Texture2D *pTextureRsc, const DepthStencilViewMode dsvMode, const int textureArrayIndex, std::optional<unsigned> face );
	//	\function	IDepthStencilView	||	\date	2024/04/25 13:34
	//	\brief	for binding depth stencil as input
	IDepthStencilView( Graphics &gfx, const unsigned width, const unsigned height, const bool bBindAsShaderInput, const DepthStencilViewMode dsvMode );
public:
	void bindRenderSurface( Graphics &gfx ) cond_noex override;
	void bindRenderSurface( Graphics &gfx, IRenderSurface *rt ) cond_noex override;
	void bindRenderSurface( Graphics &gfx, IRenderTargetView *rt ) cond_noex;
	void clear( Graphics &gfx, const std::array<float, 4> &unused = {} ) cond_noex override;
	void clean( Graphics &gfx ) cond_noex;
	//	\function	convertToBitmap	||	\date	2021/06/25 18:55
	//	\brief	copy from resource to staging --- linearize & normalize depth values by default
	const Bitmap convertToBitmap( Graphics &gfx, const unsigned textureWidth, const unsigned textureHeight, const bool bLinearize = true ) const;
	unsigned int getWidth() const noexcept;
	unsigned int getHeight() const noexcept;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& d3dResourceCom() noexcept;
	ID3D11DepthStencilView* d3dResource() const noexcept;
	virtual void setDebugObjectName( const char* name ) noexcept override;
private:
	std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC> createStagingTexture( Graphics &gfx ) const;
};

class DepthStencilShaderInput
	: public IDepthStencilView
{
	unsigned int m_slot;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pD3dSrv;
public:
	DepthStencilShaderInput( Graphics &gfx, const unsigned slot, const DepthStencilViewMode dsvMode = DepthStencilViewMode::DefaultDS );
	DepthStencilShaderInput( Graphics &gfx, const unsigned width, const unsigned height, const unsigned slot, const DepthStencilViewMode dsvMode = DepthStencilViewMode::DefaultDS );

	void bind( Graphics &gfx ) cond_noex override;
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
	DepthStencilOutput( Graphics &gfx );
	DepthStencilOutput( Graphics &gfx, const unsigned width, const unsigned height );
	DepthStencilOutput( Graphics &gfx, ID3D11Texture2D *pTexture, const DepthStencilViewMode dsvMode, std::optional<unsigned> face );
	DepthStencilOutput( Graphics &gfx, ID3D11Texture2D *pTexture, const DepthStencilViewMode dsvMode, const int textureArrayIndex, std::optional<unsigned> face );

	void bind( Graphics &gfx ) cond_noex override;
};
