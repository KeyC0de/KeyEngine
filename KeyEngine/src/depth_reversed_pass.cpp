#include "depth_reversed_pass.h"
#include "binder.h"
#include "linker.h"
#include "render_target.h"
#include "depth_stencil_view.h"
#include "depth_stencil_state.h"


namespace ren
{

DepthReversedPass::DepthReversedPass( Graphics &gph,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::DepthReversed ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
}


}//ren