#include "blur_outline_draw_pass.h"
#include "pixel_shader.h"
#include "vertex_shader.h"
#include "depth_stencil_state.h"
#include "producer.h"
#include "render_target.h"
#include "blend_state.h"


namespace ren
{

BlurOutlineDrawPass::BlurOutlineDrawPass( Graphics &gph,
	const std::string &name,
	int rezReductFactor )
	:
	RenderQueuePass{name}
{
	auto width = gph.getClientWidth() / rezReductFactor;
	auto height = gph.getClientHeight() / rezReductFactor;

	m_pRtv = std::make_unique<RenderTargetShaderInput>( gph,
		width,
		height,
		0u );
	addPassBindable( VertexShader::fetch( gph,
		"flat_vs.cso" ) );
	addPassBindable( PixelShader::fetch( gph,
		"flat_ps.cso" ) );
	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::DepthOffStencilReadFF ) );

	// m_pRtv will be bound as an Output this Pass
	// and as an input Bindable the next Pass (HorizontalBlurPass) to read from the texture
	addProducer( BindableProducer<IRenderTargetView>::make( "blurOutlineRttOut",
		m_pRtv ) );

	addPassBindable( BlendState::fetch( gph,
		BlendState::NoBlend,
		0u ) );
}

void BlurOutlineDrawPass::run( Graphics &gph ) const cond_noex
{
	m_pRtv->clear( gph );
	RenderQueuePass::run( gph );
}


}