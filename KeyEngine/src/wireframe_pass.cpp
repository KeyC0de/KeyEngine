#include "wireframe_pass.h"
#include "primitive_topology.h"
#include "binder.h"
#include "linker.h"
#include "render_target.h"
#include "depth_stencil_view.h"
#include "rasterizer_state.h"
#include "texture_sampler_state.h"
#include "depth_stencil_state.h"
#include "blend_state.h"


namespace ren
{

WireframePass::WireframePass( Graphics &gfx,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addPassBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST ) );

	addPassBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Wireframe, RasterizerState::FaceMode::Front ) );
	addPassBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::Default ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
}


}//ren