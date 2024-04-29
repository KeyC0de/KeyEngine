#include "depth_reversed_pass.h"
#include "primitive_topology.h"
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
	addPassBindable( PrimitiveTopology::fetch( gph, D3D11_PRIMITIVE_TOPOLOGY_LINELIST ) );

	addPassBindable( DepthStencilState::fetch( gph, DepthStencilState::Mode::DepthReversedStencilOff ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
}


}//ren