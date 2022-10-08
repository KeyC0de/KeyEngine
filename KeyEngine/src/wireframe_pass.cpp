#include "wireframe_pass.h"
#include "consumer.h"
#include "producer.h"
#include "render_target.h"
#include "depth_stencil_view.h"
#include "rasterizer_state.h"
#include "texture_sampler_state.h"
#include "depth_stencil_state.h"
#include "blend_state.h"


namespace ren
{

WireframePass::WireframePass( Graphics &gph,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addPassBindable( RasterizerState::fetch( gph,
		RasterizerState::FrontSided,
		RasterizerState::Solid ) );
	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::Default ) );

	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

	addProducer( RenderSurfaceProducer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
}


}//ren