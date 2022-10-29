#include "blur_pass.h"
#include "pixel_shader.h"
#include "texture_sampler_state.h"
#include "texture.h"
#include "blend_state.h"


namespace ren
{

BlurPass::BlurPass( Graphics &gph,
	const std::string &name )
	:
	IFullscreenPass{gph, name}
{
	const unsigned rezReductFactor = 1u;
	const unsigned width = gph.getClientWidth() / rezReductFactor;
	const unsigned height = gph.getClientHeight() / rezReductFactor;
	// create a RTV to render the scene to an offscreen texture, and next Pass we'll read it
	//m_pRtv = std::make_shared<RenderTargetShaderInput>( gph,
		//width,
		//height,
		//0u );

	//m_pOffscreenPostProcessTex = std::make_shared<TextureOffscreenRT>( gph,
	//	width,
	//	height,
	//	0u );

	//addPassBindable( std::make_shared<TextureOffscreenRT>( gph,
		//width,
		//height,
		//0u ) );

	addPassBindable( TextureSamplerState::fetch( gph,
		0u,
		TextureSamplerState::FilterMode::Anisotropic,
		TextureSamplerState::AddressMode::Wrap ) );

	addPassBindable( PixelShader::fetch( gph,
		"blur_ps.cso" ) );
	//addPassBindable( BlendState::fetch( gph,
		//BlendState::Mode::Additive,
		//0u ) );

	//addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget",
		//m_pRtv ) );
	//addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil",
		//m_pDsv ) );
	// read from the offscreen texture and Blur it
	//addContainerBindableBinder<TextureOffscreenRT>( "offscreenPostProcessIn" );
	//addContainerBindableBinder<IRenderTargetView>( "offscreenPostProcessIn" );

	//addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget",
		//m_pRtv ) );
	//addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil",
		//m_pDsv ) );
}

void BlurPass::reset() cond_noex
{
	pass_;
}


}// namespace ren