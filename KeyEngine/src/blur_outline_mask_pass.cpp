#include "blur_outline_mask_pass.h"
#include "pixel_shader_null.h"
#include "vertex_shader.h"
#include "depth_stencil_state.h"


namespace ren
{

BlurOutlineMaskPass::BlurOutlineMaskPass( Graphics &gph,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
	addProducer( RenderSurfaceProducer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
	addPassBindable( VertexShader::fetch( gph,
		"flat_vs.cso" ) );
	addPassBindable( PixelShaderNull::fetch( gph ) );
	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::DepthOffStencilWriteFF ) );
}


}// ren