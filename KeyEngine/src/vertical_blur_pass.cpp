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
	addPassBindable( PixelShader::fetch( gph,
		"blur_separ_ps.cso" ) );

	addPassBindable( TextureSamplerState::fetch( gph,
		0u,
		TextureSamplerState::FilterMode::Trilinear,
		TextureSamplerState::AddressMode::Clamp ) );

	addPassBindable( BlendState::fetch( gph,
		BlendState::Mode::Alpha,
		0u ) );

	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
	addConsumer( BindableConsumer<PixelShaderConstantBufferEx>::make( "blurDirection",
		m_pPscbBlurDirection ) );
	addContainerBindableConsumer<IRenderTargetView>( "offscreenBlurOutlineIn" );
	addContainerBindableConsumer<PixelShaderConstantBufferEx>( "blurKernel" );

	addProducer( RenderSurfaceProducer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addProducer( RenderSurfaceProducer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
}

void VerticalBlurPass::run( Graphics &gph ) const cond_noex
{
	auto pcBuf = m_pPscbBlurDirection->getBuffer();
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