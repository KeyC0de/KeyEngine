#include "render_surface_clear_pass.h"
#include "binder.h"


namespace ren
{

RenderSurfaceClearPass::RenderSurfaceClearPass( const std::string &name )
	:
	IPass{name}
{
	addBinder( RenderSurfaceBinder<IRenderSurface>::make( "buffer", m_pRenderSurface ) );
	addLinker( RenderSurfaceLinker<IRenderSurface>::make( "buffer", m_pRenderSurface ) );
}

void RenderSurfaceClearPass::run( Graphics &gfx ) const cond_noex
{
	m_pRenderSurface->clear( gfx );
}

void RenderSurfaceClearPass::reset() cond_noex
{
	pass_;
}


}//ren