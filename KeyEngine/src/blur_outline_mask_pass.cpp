#include "blur_outline_mask_pass.h"
#include "pixel_shader_null.h"
#include "vertex_shader.h"
#include "depth_stencil_state.h"


namespace ren
{

BlurOutlineMaskPass::BlurOutlineMaskPass( Graphics& gph,
	const std::string& name )
	:
	RenderQueuePass{name}
{
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
	addProducer( RenderSurfaceProducer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
	addPassSharedBindable( VertexShader::fetch( gph,
		"flat_vs.cso" ) );
	addPassSharedBindable( PixelShaderNull::fetch( gph ) );
	addPassSharedBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::DepthOffStencilWriteFF ) );
}


}// ren