#include "horizontal_blur_pass.h"
#include "pixel_shader.h"
#include "blend_state.h"
#include "texture_sampler.h"


namespace ren
{

HorizontalBlurPass::HorizontalBlurPass( Graphics& gph,
	const std::string& name,
	int rezReductFactor )
	:
	FullscreenPass{gph, name}
{
	auto width = gph.getClientWidth() / rezReductFactor;
	auto height = gph.getClientHeight() / rezReductFactor;

	addPassSharedBindable( PixelShader::fetch( gph,
		"blur_separ_ps.cso" ) );

	addPassSharedBindable( TextureSampler::fetch( gph,
		0u,
		TextureSampler::Type::Trilinear,
		false,
		true ) );
	
	addContainerBindableConsumer<IRenderTargetView>( "blurRttIn" );
	addContainerBindableConsumer<PixelConstantBufferEx>( "blurKernel" );
	addConsumer( BindableConsumer<PixelConstantBufferEx>::make( "blurDirection",
		m_pPcbBlurDirection ) );

	m_pRtv = std::make_shared<RenderTargetShaderInput>( gph,
		width,
		height,
		0u );
	addProducer( BindableProducer<IRenderTargetView>::make( "blurRttOut",
		m_pRtv ) );

	addPassSharedBindable( BlendState::fetch( gph,
		BlendState::NoBlend,
		0u ) );
}

void HorizontalBlurPass::run( Graphics& gph ) const cond_noex
{
	auto pcb = m_pPcbBlurDirection->getBuffer();
	pcb["bHorizontal"] = true;
	m_pPcbBlurDirection->setBuffer( pcb );
	m_pPcbBlurDirection->bind( gph );
	FullscreenPass::run( gph );
}

void HorizontalBlurPass::reset() cond_noex
{
	pass_;
}


}//ren