#include "render_surface_clear_pass.h"
#include "consumer.h"


namespace ren
{

RenderSurfaceClearPass::RenderSurfaceClearPass( const std::string &name )
	:
	IPass{name}
{
	addConsumer( RenderSurfaceConsumer<IRenderSurface>::make( "buffer",
		m_pBuffer ) );
	addProducer( RenderSurfaceProducer<IRenderSurface>::make( "buffer",
		m_pBuffer ) );
}

void RenderSurfaceClearPass::run( Graphics &gph ) const cond_noex
{
	m_pBuffer->clear( gph );
}

void RenderSurfaceClearPass::reset() cond_noex
{
	pass_;
}


}//ren