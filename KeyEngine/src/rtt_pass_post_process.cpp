#include "rtt_pass_post_process.h"
#include "pixel_shader.h"
#include "depth_stencil_state.h"
#include "texture_sampler_state.h"


namespace ren
{

BasePostProcessingPass::BasePostProcessingPass( Graphics &gph,
	const std::string &name,
	const unsigned rezReductFactor )
	:
	//IPass{name}
	//IBindablePass{name}
	IFullscreenPass{gph, name}
{
	//addPassBindable( PixelShader::fetch( gph,
		//"blur_ps.cso" ) );
	//addPassBindable( TextureSamplerState::fetch( gph,
		//0u,
		//TextureSamplerState::FilterMode::Anisotropic,
		//TextureSamplerState::AddressMode::Wrap ) );
	//addPassBindable( PixelShaderNull::fetch( gph ) );

	//addPassBindable( PixelShader::fetch( gph,
		//"passthrough_ps.cso" ) );

	//addPassBindable( DepthStencilState::fetch( gph,
		//DepthStencilState::Mode::Default) );

	//addPassBindable( TextureSamplerState::fetch( gph,
		//0u,
		//TextureSamplerState::FilterMode::Trilinear,
		//TextureSamplerState::AddressMode::Clamp ) );

	const unsigned width = gph.getClientWidth() / rezReductFactor;
	const unsigned height = gph.getClientHeight() / rezReductFactor;
	// create a RTV to render the scene to an offscreen texture, and next Pass we'll read it
	m_pRtv = std::make_shared<RenderTargetShaderInput>( gph,
		width,
		height,
		4u );
	addLinker( BindableLinker<IRenderTargetView>::make( "offscreenPostProcessOut",
		m_pRtv ) );

	// This Pass: create a texture, create a RTV to that texture, call OMSetRenderTargets
	// Next Pass: read from the texture using SRV
	//m_pOffscreenPostProcessTex = std::make_shared<TextureOffscreenRT>( gph,
		//width,
		//height,
		//4u );

	//addLinker( BindableLinker<TextureOffscreenRT>::make( "offscreenPostProcessOut",
		//m_pOffscreenPostProcessTex ) );
	//m_pRtv = m_pOffscreenPostProcessTex->shareRenderTarget();
}

void BasePostProcessingPass::run( Graphics &gph ) const cond_noex
{
	//m_pRtv->clear( gph );
	//m_pRtv->bind( gph );
	IFullscreenPass::run( gph );
	//bind( gph );
}

void BasePostProcessingPass::reset() cond_noex
{
	pass_;
}


}//ren