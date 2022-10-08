#include "depth_reversed_pass.h"
#include "consumer.h"
#include "producer.h"
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

	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

	addProducer( RenderSurfaceProducer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addProducer( RenderSurfaceProducer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
}


}//ren