#include "rtt_pass_post_process.h"
#include "pixel_shader.h"


namespace ren
{

RttPassForPostProcessing::RttPassForPostProcessing( Graphics &gph,
	const std::string &name,
	const unsigned rezReductFactor )
	:
	IFullscreenPass{gph, name}
{
	//addPassBindable( PixelShaderNull::fetch( gph ) );

	const unsigned width = gph.getClientWidth() / rezReductFactor;
	const unsigned height = gph.getClientHeight() / rezReductFactor;
	// create a RTV to render the scene to an offscreen texture, and next Pass we'll read it
	//m_pRtv = std::make_unique<RenderTargetShaderInput>( gph,
		//width,
		//height,
		//4u );
	//addLinker( BindableLinker<IRenderTargetView>::make( "offscreenPostProcessOut",
		//m_pRtv ) );

	// create the offscreen texture
	//m_pOffscreenDsvCubemap = std::make_shared<CubeTextureOffscreenDS>( gph,
		//s_shadowMapResolution,
		//s_shadowMapResolution,
		//3u,
		//DepthStencilViewMode::ShadowDepth );

	//addLinker( BindableLinker<CubeTextureOffscreenDS>::make( "offscreenShadowCubemapOut",
		//m_pOffscreenDsvCubemap ) );

	m_pOffscreenPostProcessTex = std::make_shared<TextureOffscreenRT>( gph,
		width,
		height,
		4u );

	addLinker( BindableLinker<TextureOffscreenRT>::make( "offscreenPostProcessOut",
		m_pOffscreenPostProcessTex ) );
	m_pRtv = m_pOffscreenPostProcessTex->shareRenderTarget();
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