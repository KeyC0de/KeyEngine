#include "blur_outline_draw_pass.h"
#include "pixel_shader.h"
#include "vertex_shader.h"
#include "depth_stencil_state.h"
#include "producer.h"
#include "render_target.h"
#include "blend_state.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"


namespace ren
{

BlurOutlineDrawPass::BlurOutlineDrawPass( Graphics &gph,
	const std::string &name,
	const unsigned rezReductFactor )
	:
	RenderQueuePass{name}
{
	addPassBindable( VertexShader::fetch( gph,
		"flat_vs.cso" ) );
	addPassBindable( PixelShader::fetch( gph,
		"flat_ps.cso" ) );
	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::DepthOffStencilReadFF ) );
	addPassBindable( BlendState::fetch( gph,
		BlendState::NoBlend,
		0u ) );

	const unsigned width = gph.getClientWidth() / rezReductFactor;
	const unsigned height = gph.getClientHeight() / rezReductFactor;
	// create a SRV to read the main texture
	m_pRtv = std::make_shared<RenderTargetShaderInput>( gph,
		width,
		height,
		0u );
	addProducer( BindableProducer<IRenderTargetView>::make( "offscreenBlurOutlineOut",
		m_pRtv ) );
}

void BlurOutlineDrawPass::run( Graphics &gph ) const cond_noex
{
	m_pRtv->clear( gph );
	RenderQueuePass::run( gph );
}


}