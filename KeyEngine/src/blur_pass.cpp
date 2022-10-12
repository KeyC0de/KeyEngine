#include "blur_pass.h"
#include "pixel_shader.h"
#include "texture_sampler_state.h"
#include "blend_state.h"


namespace ren
{

BlurPass::BlurPass( Graphics &gph,
	const std::string &name )
	:
	IFullscreenPass{gph, name}
{
	addPassBindable( PixelShader::fetch( gph,
		"blur_ps.cso" ) );
	addPassBindable( TextureSamplerState::fetch( gph,
		4u,
		TextureSamplerState::FilterMode::Trilinear,
		TextureSamplerState::AddressMode::Clamp ) );
	//addPassBindable( BlendState::fetch( gph,
		//BlendState::Mode::Multiplicative,
		//0u ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
	// read from the offscreen texture and Blur it
	addContainerBindableBinder<IRenderTargetView>( "offscreenPostProcessIn" );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
}

void BlurPass::reset() cond_noex
{
	pass_;
}


}// namespace ren