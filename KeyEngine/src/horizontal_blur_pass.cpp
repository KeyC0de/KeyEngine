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

	addPassBindable( PixelShader::fetch( gph,
		"blur_separ_ps.cso" ) );

	addPassBindable( TextureSampler::fetch( gph,
		0u,
		TextureSampler::FilterMode::Trilinear,
		TextureSampler::AddressMode::Clamp ) );
	
	addContainerBindableConsumer<IRenderTargetView>( "blurRttIn" );
	addContainerBindableConsumer<PixelShaderConstantBufferEx>( "blurKernel" );
	addConsumer( BindableConsumer<PixelShaderConstantBufferEx>::make( "blurDirection",
		m_pPscbBlurDirection ) );

	m_pRtv = std::make_shared<RenderTargetShaderInput>( gph,
		width,
		height,
		0u );
	addProducer( BindableProducer<IRenderTargetView>::make( "blurRttOut",
		m_pRtv ) );

	addPassBindable( BlendState::fetch( gph,
		BlendState::NoBlend,
		0u ) );
}

void HorizontalBlurPass::run( Graphics& gph ) const cond_noex
{
	auto pscb = m_pPscbBlurDirection->getBuffer();
	pscb["bHorizontal"] = true;
	m_pPscbBlurDirection->setBuffer( pscb );
	m_pPscbBlurDirection->bind( gph );
	FullscreenPass::run( gph );
}

void HorizontalBlurPass::reset() cond_noex
{
	pass_;
}


}// namespace ren