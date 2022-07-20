#include "wireframe_pass.h"
#include "consumer.h"
#include "producer.h"
#include "render_target.h"
#include "depth_stencil_view.h"
#include "rasterizer.h"


namespace ren
{

WireframePass::WireframePass( Graphics &gph,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
	addProducer( RenderSurfaceProducer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addPassBindable( Rasterizer::fetch( gph,
		false,
		true ) );
}


}//ren