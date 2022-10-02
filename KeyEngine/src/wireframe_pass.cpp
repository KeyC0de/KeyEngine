#include "wireframe_pass.h"
#include "consumer.h"
#include "producer.h"
#include "render_target.h"
#include "depth_stencil_view.h"
#include "rasterizer_state.h"
#include "texture_sampler_state.h"
#include "depth_stencil_state.h"


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
	addProducer( RenderSurfaceProducer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

	addPassBindable( std::make_shared<TextureSamplerState>( gph,
		0u,
		TextureSamplerState::FilterMode::Anisotropic,
		TextureSamplerState::AddressMode::Wrap ) );

	addPassBindable( RasterizerState::fetch( gph,
		RasterizerState::FrontSided,
		RasterizerState::Wireframe ) );

	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::Default ) );
}


}//ren