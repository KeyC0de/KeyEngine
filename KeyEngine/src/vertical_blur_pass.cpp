#include "vertical_blur_pass.h"
#include "pixel_shader.h"
#include "blend_state.h"
#include "texture_sampler_state.h"


namespace ren
{

VerticalBlurPass::VerticalBlurPass( Graphics &gph,
	const std::string &name )
	:
	IFullscreenPass{gph, name}
{
	addPassBindable( PixelShader::fetch( gph, "blur_separ_ps.cso" ) );
	addPassBindable( TextureSamplerState::fetch( gph, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Trilinear, TextureSamplerState::AddressMode::Clamp ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
	addBinder( Binder<PixelShaderConstantBufferEx>::make( "blurDirection", m_pPscbBlurDirection ) );
	// read from offscreen texture and Vertically Blur it (separated filter)
	addContainerBindableBinder<IRenderTargetView>( "offscreenBlurOutlineIn" );
	addContainerBindableBinder<PixelShaderConstantBufferEx>( "blurKernel" );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
}

void VerticalBlurPass::run( Graphics &gph ) const cond_noex
{
	auto pcBuf = m_pPscbBlurDirection->getBufferCopy();
	pcBuf["bHorizontal"] = false;
	m_pPscbBlurDirection->setBuffer( pcBuf );
	m_pPscbBlurDirection->bind( gph );
	IFullscreenPass::run( gph );
}

void VerticalBlurPass::reset() cond_noex
{
	pass_;
}


}