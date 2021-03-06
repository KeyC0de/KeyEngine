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

	std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC>
		createStagingTexture( Graphics &gph ) const;
protected:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pDsv;
	unsigned int m_width;
	unsigned int m_height;
public:
	enum Mode
	{
		Normal,
		ShadowDepth,	// for a shadow buffer 32bit float ZB - no SB
	};
protected:
	IDepthStencilView( Graphics &gph, Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture,
		unsigned face );
	IDepthStencilView( Graphics &gph, unsigned width, unsigned height,
		bool bBindAsShaderInput, Mode mode );
public:
	void bindRenderSurface( Graphics &gph ) cond_noex override;
	void bindRenderSurface( Graphics &gph, IRenderSurface *rt ) cond_noex override;
	void bindRenderSurface( Graphics &gph, IRenderTargetView *rt ) cond_noex;
	void clear( Graphics &gph, const std::array<float, 4>& unused = {} ) cond_noex override;
	//===================================================
	//	\function	convertToBitmap
	//	\brief  copy from resource to staging
	//			linearize & normalize depth values by default
	//	\date	2021/06/25 18:55
	Bitmap convertToBitmap( Graphics &gph, bool bLinearize = true ) const;
	unsigned int getWidth() const noexcept;
	unsigned int getHeight() const noexcept;
};

class DepthStencilShaderInput
	: public IDepthStencilView
{
	unsigned int m_slot;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSrv;
public:
	DepthStencilShaderInput( Graphics &gph, unsigned slot,
		Mode mode = Mode::Normal );
	DepthStencilShaderInput( Graphics &gph, unsigned width, unsigned height,
		unsigned slot, Mode mode = Mode::Normal );

	void bind( Graphics &gph ) cond_noex override;
};

//=============================================================
//	\class	DepthStencilOutput
//
//	\author	KeyC0de
//	\date	2021/10/19 0:47
//
//	\brief	depth stencil view used for regular rendering,
//				or Rendering to a Texture (RTT) (offscreen buffer)
//=============================================================
class DepthStencilOutput
	: public IDepthStencilView
{
public:
	DepthStencilOutput( Graphics &gph, Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture,
		unsigned face );
	DepthStencilOutput( Graphics &gph );
	DepthStencilOutput( Graphics &gph, unsigned width, unsigned height );

	void bind( Graphics &gph ) cond_noex override;
};