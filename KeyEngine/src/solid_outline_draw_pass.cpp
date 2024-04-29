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
	addPassBindable( VertexShader::fetch( gph, "flat_vs.cso" ) );
	addPassBindable( PixelShader::fetch( gph, "flat_ps.cso" ) );
	addPassBindable( DepthStencilState::fetch( gph, DepthStencilState::Mode::DepthOffStencilReadFF ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
}


}