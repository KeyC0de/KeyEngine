#include "transparent_pass.h"
#include "primitive_topology.h"
#include "pixel_shader.h"
#include "vertex_shader.h"
#include "depth_stencil_state.h"
#include "linker.h"
#include "render_target.h"
#include "rasterizer_state.h"
#include "blend_state.h"


namespace ren
{

TransparentPass::TransparentPass( Graphics &gfx,
	const std::string &name )
	:
	RenderQueuePass{name, {}, true}
{
	//addPassBindable( BlendState::fetch( gfx, BlendState::Mode::Alpha, 0u ) );

	addPassBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	addPassBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );
	addPassBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::Default ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
}

}