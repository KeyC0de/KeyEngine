#include "vertical_blur_pass.h"
#include "pixel_shader.h"
#include "blend_state.h"
#include "texture_sampler.h"


namespace ren
{

VerticalBlurPass::VerticalBlurPass( Graphics &gph,
	const std::string &name )
	:
	FullscreenPass{gph, name}
{
	addPassBindable( PixelShader::fetch( gph,
		"blur_separ_ps.cso" ) );

	addPassBindable( TextureSampler::fetch( gph,
		0u,
		TextureSampler::FilterMode::Trilinear,
		TextureSampler::AddressMode::Clamp ) );

	addPassBindable( BlendState::fetch( gph,
		BlendState::Alpha,
		0u ) );

	addContainerBindableConsumer<IRenderTargetView>( "blurRttIn" );
	addContainerBindableConsumer<PixelShaderConstantBufferEx>( "blurKernel" );
	addConsumer( BindableConsumer<PixelShaderConstantBufferEx>::make( "blurDirection",
		m_pPscbBlurDirection ) );
	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

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
	FullscreenPass::run( gph );
}

void VerticalBlurPass::reset() cond_noex
{
	pass_;
}


}