#include "blur_pass.h"
#include "pixel_shader.h"
#include "blend_state.h"
#include "texture_sampler_state.h"
#include "depth_stencil_state.h"
#include "rasterizer_state.h"


namespace ren
{
// #FIXME: Setup BlurPass - Graphics Debugging
BlurPass::BlurPass( Graphics &gph,
	const std::string &name )
	:
	FullscreenPass{gph, name}
{
	addPassBindable( RasterizerState::fetch( gph,
		RasterizerState::FrontSided,
		RasterizerState::Solid ) );

	addPassBindable( BlendState::fetch( gph,
		BlendState::Mode::Additive,
		0u ) );
	
	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::Default ) );

	addPassBindable( TextureSamplerState::fetch( gph,
		0u,
		TextureSamplerState::FilterMode::Trilinear,
		TextureSamplerState::AddressMode::Clamp ) );

	addPassBindable( PixelShader::fetch( gph,
		"blur_ps.cso" ) );

	//addPassBindable( PixelShaderNull::fetch( gph ) );

	// bind RT as Shader Input to read from it
	auto rezReductFactor = 2;
	const unsigned width = gph.getClientWidth() / rezReductFactor;
	const unsigned height = gph.getClientHeight() / rezReductFactor;
	
	//m_pRtv = std::make_shared<RenderTargetShaderInput>( gph,
	//	width,
	//	height,
	//	0u );

	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

	addProducer( RenderSurfaceProducer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addProducer( RenderSurfaceProducer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
}

void BlurPass::reset() cond_noex
{
	pass_;
}


}// namespace ren