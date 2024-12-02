#pragma once

#include <optional>
#include "key_wrl.h"
#include "bindable.h"
#include "render_surface.h"
#include "texture_desc.h"


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
	/// \brief	For creating an output RT only
	IRenderTargetView( Graphics &gfx, ID3D11Texture2D *pTexture, std::optional<unsigned> face );
	/// \brief	Render to Texture constructor
	IRenderTargetView( Graphics &gfx, const unsigned width, const unsigned height );
public:
	/// \brief	unbind targets from Output - currently UNUSED
	static void unbind( Graphics &gfx ) noexcept;
public:
	void bindRenderSurface( Graphics &gfx ) cond_noex override;
	void bindRenderSurface( Graphics &gfx, IRenderSurface *pRs ) cond_noex override;
	void bindRenderSurface( Graphics &gfx, IDepthStencilView *pDepthStencilView ) cond_noex;
	void clear( Graphics &gfx, const std::array<float, 4> &color = {0.0f, 0.0f, 0.0f, 0.0f} ) cond_noex override;
	void clean( Graphics &gfx ) cond_noex;
	const Bitmap convertToBitmap( Graphics &gfx, const unsigned textureWidth, const unsigned textureHeight ) const;
	unsigned getWidth() const noexcept;
	unsigned getHeight() const noexcept;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& d3dResourceCom() noexcept;
	ID3D11RenderTargetView* d3dResource() const noexcept;
	virtual void setDebugObjectName( const char* name ) noexcept override;
private:
	/// \brief	create a texture resource compatible with our RTV, but with Staging usage (CPU read access, no GPU access)
	std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC> createStagingTexture( Graphics &gfx ) const;
	void bindRenderSurface( Graphics &gfx, ID3D11DepthStencilView *pD3dDsv ) cond_noex;
};

///=============================================================
/// \class	RenderTargetShaderInput
/// \author	KeyC0de
/// \date	2022/10/13 11:09
/// \brief	RTV wrapper that will be used for Input from the Back Buffer only
///=============================================================
class RenderTargetShaderInput
	: public IRenderTargetView
{
	unsigned m_slot;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pD3dSrv;
public:
	RenderTargetShaderInput( Graphics &gfx, const unsigned width, const unsigned height, const unsigned slot, const RenderTargetViewMode rtvMode = DefaultRT );

	void bind( Graphics &gfx ) cond_noex override;
	unsigned getSlot() const noexcept;
};

///=============================================================
/// \class	RenderTargetOutput
/// \author	KeyC0de
/// \date	2021/10/19 0:47
/// \brief	RTV wrapper that will be used for Output only
///=============================================================
class RenderTargetOutput
	: public IRenderTargetView
{
public:
	RenderTargetOutput( Graphics &gfx, ID3D11Texture2D *pTexture, std::optional<unsigned> face = {} );

	void bind( Graphics &gfx ) cond_noex override;
};