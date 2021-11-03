#include "vertical_blur_pass.h"
#include "pixel_shader.h"
#include "blend_state.h"
#include "texture_sampler.h"


namespace ren
{

VerticalBlurPass::VerticalBlurPass( Graphics& gph,
	const std::string& name )
	:
	FullscreenPass{gph, name}
{
	addPassSharedBindable( PixelShader::fetch( gph,
		"blur_separ_ps.cso" ) );

	addPassSharedBindable( TextureSampler::fetch( gph,
		0u,
		TextureSampler::Type::Trilinear,
		false,
		true ) );

	addContainerBindableConsumer<IRenderTargetView>( "blurOutlineRttIn" );
	addContainerBindableConsumer<PixelConstantBufferEx>( "blurKernel" );
	addConsumer( BindableConsumer<PixelConstantBufferEx>::make( "blurDirection",
		m_pPcbBlurDirection ) );
	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

	addProducer( RenderSurfaceProducer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addProducer( RenderSurfaceProducer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

	addPassSharedBindable( BlendState::fetch( gph,
		BlendState::Alpha,
		0u ) );
}

void VerticalBlurPass::run( Graphics& gph ) const cond_noex
{
	auto pcBuf = m_pPcbBlurDirection->getBuffer();
	pcBuf["bHorizontal"] = false;
	m_pPcbBlurDirection->setBuffer( pcBuf );
	m_pPcbBlurDirection->bind( gph );
	FullscreenPass::run( gph );
}

void VerticalBlurPass::reset() cond_noex
{
	pass_;
}


}