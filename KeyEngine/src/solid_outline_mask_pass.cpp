#include "solid_outline_mask_pass.h"
#include <string>
#include "depth_stencil_state.h"
#include "vertex_shader.h"
#include "pixel_shader_null.h"
#include "blend_state.h"


namespace ren
{

SolidOutlineMaskPass::SolidOutlineMaskPass( Graphics &gph,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addPassBindable( VertexShader::fetch( gph,
		"flat_vs.cso" ) );

	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::DepthOffStencilWriteFF ) );

	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

	addProducer( RenderSurfaceProducer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addProducer( RenderSurfaceProducer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
	
	addPassBindable( PixelShaderNull::fetch( gph ) );

	addPassBindable( BlendState::fetch( gph,
		BlendState::NoBlend,
		0u ) );
}


}