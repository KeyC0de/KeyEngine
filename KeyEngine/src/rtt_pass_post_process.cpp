#include "rtt_pass_post_process.h"
#include "pixel_shader.h"
#include "depth_stencil_state.h"


namespace ren
{

RttPassForPostProcessing::RttPassForPostProcessing( Graphics &gph,
	const std::string &name,
	const unsigned rezReductFactor )
	:
	IFullscreenPass{gph, name}
{
	addPassBindable( PixelShaderNull::fetch( gph ) );
	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::Default ) );

	const unsigned width = gph.getClientWidth() / rezReductFactor;
	const unsigned height = gph.getClientHeight() / rezReductFactor;
	// create a RTV to render the scene to an offscreen texture, and next Pass we'll read it
/*	m_pRtv = std::make_unique<RenderTargetShaderInput>( gph,
		width,
		height,
		4u );
	addLinker( BindableLinker<IRenderTargetView>::make( "offscreenPostProcessOut",
		m_pRtv ) );*/

	m_pOffscreenPostProcessTex = std::make_shared<Texture>( gph,
		width * height,
		4u );

	addLinker( BindableLinker<Texture>::make( "offscreenPostProcessOut",
		m_pOffscreenPostProcessTex ) );
	m_pRtv = m_pOffscreenPostProcessTex->sha;
}

void RttPassForPostProcessing::run( Graphics &gph ) const cond_noex
{
	m_pRtv->clear( gph );
	IFullscreenPass::run( gph );
}

void RttPassForPostProcessing::reset() cond_noex
{
	pass_;
}


}//ren