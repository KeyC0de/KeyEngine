#include "horizontal_blur_pass.h"
#include "pixel_shader.h"
#include "blend_state.h"
#include "texture_sampler_state.h"


namespace ren
{

HorizontalBlurPass::HorizontalBlurPass( Graphics &gph,
	const std::string &name,
	const unsigned rezReductFactor )
	:
	IFullscreenPass{gph, name}
{
	addPassBindable( PixelShader::fetch( gph,
		"blur_separ_ps.cso" ) );

	addPassBindable( TextureSamplerState::fetch( gph,
		0u,
		TextureSamplerState::FilterMode::Trilinear,
		TextureSamplerState::AddressMode::Clamp ) );

	//addPassBindable( BlendState::fetch( gph,
		//BlendState::Mode::Alpha,
		//0u ) );

	addConsumer( BindableConsumer<PixelShaderConstantBufferEx>::make( "blurDirection",
		m_pPscbBlurDirection ) );
	addContainerBindableConsumer<IRenderTargetView>( "offscreenBlurOutlineIn" );
	addContainerBindableConsumer<PixelShaderConstantBufferEx>( "blurKernel" );

	const unsigned width = gph.getClientWidth() / rezReductFactor;
	const unsigned height = gph.getClientHeight() / rezReductFactor;
	// create a SRV to read the main texture and perform the Horizontal Blur
	m_pRtv = std::make_shared<RenderTargetShaderInput>( gph,
		width,
		height,
		0u );
	addProducer( BindableProducer<IRenderTargetView>::make( "offscreenBlurOutlineOut",
		m_pRtv ) );
}

void HorizontalBlurPass::run( Graphics &gph ) const cond_noex
{
	auto pscb = m_pPscbBlurDirection->getBuffer();
	pscb["bHorizontal"] = true;
	m_pPscbBlurDirection->setBuffer( pscb );
	m_pPscbBlurDirection->bind( gph );
	IFullscreenPass::run( gph );
}

void HorizontalBlurPass::reset() cond_noex
{
	pass_;
}


}// namespace ren