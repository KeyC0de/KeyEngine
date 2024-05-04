#include "vertical_blur_pass.h"
#include "pixel_shader.h"
#include "blend_state.h"
#include "texture_sampler_state.h"


namespace ren
{

VerticalBlurPass::VerticalBlurPass( Graphics &gfx,
	const std::string &name )
	:
	IFullscreenPass{gfx, name}
{
	addPassBindable( PixelShader::fetch( gfx, "blur_separ_ps.cso" ) );
	addPassBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Trilinear, TextureSamplerState::AddressMode::Clamp ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
	addBinder( Binder<PixelShaderConstantBufferEx>::make( "blurDirection", m_pPscbBlurDirection ) );
	// read from offscreen texture and Vertically Blur it (separated filter)
	addContainerBindableBinder<IRenderTargetView>( "offscreenBlurOutlineIn" );
	addContainerBindableBinder<PixelShaderConstantBufferEx>( "blurKernel" );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
}

void VerticalBlurPass::run( Graphics &gfx ) const cond_noex
{
	auto pcBuf = m_pPscbBlurDirection->getBufferCopy();
	pcBuf["bHorizontal"] = false;
	m_pPscbBlurDirection->setBuffer( pcBuf );
	m_pPscbBlurDirection->bind( gfx );
	IFullscreenPass::run( gfx );
}

void VerticalBlurPass::reset() cond_noex
{
	pass_;
}


}