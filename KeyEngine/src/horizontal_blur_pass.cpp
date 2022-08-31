#include "horizontal_blur_pass.h"
#include "pixel_shader.h"
#include "blend_state.h"
#include "texture_sampler_state.h"


namespace ren
{

HorizontalBlurPass::HorizontalBlurPass( Graphics &gph,
	const std::string &name,
	const int rezReductFactor )
	:
	FullscreenPass{gph, name}
{
	auto width = gph.getClientWidth() / rezReductFactor;
	auto height = gph.getClientHeight() / rezReductFactor;

	addPassBindable( PixelShader::fetch( gph,
		"blur_separ_ps.cso" ) );

	addPassBindable( TextureSamplerState::fetch( gph,
		0u,
		TextureSamplerState::FilterMode::Trilinear,
		TextureSamplerState::AddressMode::Clamp ) );

	addPassBindable( BlendState::fetch( gph,
		BlendState::NoBlend,
		0u ) );

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
}

void HorizontalBlurPass::run( Graphics &gph ) const cond_noex
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