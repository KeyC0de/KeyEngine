#include "solid_outline_draw_pass.h"
#include <string>
#include "depth_stencil_state.h"
#include "vertex_shader.h"
#include "pixel_shader.h"


namespace ren
{

SolidOutlineDrawPass::SolidOutlineDrawPass( Graphics &gfx,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addPassBindable( VertexShader::fetch( gfx, "flat_vs.cso" ) );
	addPassBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );
	addPassBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::DepthOffStencilReadFF ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
}


}