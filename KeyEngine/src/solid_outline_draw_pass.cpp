#include "solid_outline_draw_pass.h"
#include <string>
#include "depth_stencil_state.h"
#include "vertex_shader.h"
#include "pixel_shader.h"


namespace ren
{

SolidOutlineDrawPass::SolidOutlineDrawPass( Graphics &gph,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addPassBindable( VertexShader::fetch( gph,
		"flat_vs.cso" ) );

	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

	addProducer( RenderSurfaceProducer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addProducer( RenderSurfaceProducer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

	addPassBindable( PixelShader::fetch( gph,
		"flat_ps.cso" ) );

	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::DepthOffStencilReadFF ) );
}


}