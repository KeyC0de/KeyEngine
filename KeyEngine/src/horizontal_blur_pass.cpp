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

	addBinder( Binder<PixelShaderConstantBufferEx>::make( "blurDirection",
		m_pPscbBlurDirection ) );
	// read from offscreen texture and Horizontally Blur it (separated filter)
	addContainerBindableBinder<IRenderTargetView>( "offscreenBlurOutlineIn" );
	addContainerBindableBinder<PixelShaderConstantBufferEx>( "blurKernel" );

	const unsigned width = gph.getClientWidth() / rezReductFactor;
	const unsigned height = gph.getClientHeight() / rezReductFactor;
	// create a RTV to write (the PS operation - which performs the Horizontal Blur) to an offscreen texture, next Pass we'll read from the offscreen tex (again)
	m_pRtv = std::make_shared<RenderTargetShaderInput>( gph,
		width,
		height,
		0u );
	addLinker( BindableLinker<IRenderTargetView>::make( "offscreenBlurOutlineOut",
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