#include "transparent_pass.h"
#include "camera.h"
#include "pixel_shader.h"
#include "vertex_shader.h"
#include "depth_stencil_state.h"
#include "linker.h"
#include "render_target.h"


namespace ren
{

TransparentPass::TransparentPass( Graphics &gfx,
	const std::string &name )
	:
	RenderQueuePass{name, {}, true}
{
	addBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::DepthReadOnlyStencilOff ) );

	// TextureSamplerState is set per mesh depending on whether the mesh has a texture

	// RasterizerState is set per mesh depending on whether the mesh's texture has an alpha channel
	// by default you Cull_Back like Opaque meshes; or use CULL_NONE for double sided transparency (may be useful for very thin meshes, like leaves, or billboards)

	// BlendState is set per mesh depending on the level of transparency of the mesh (determined by its flat color's Alpha value or its texture alpha value)

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
}

void TransparentPass::run( Graphics &gfx ) const cond_noex
{
	ASSERT( m_pActiveCamera, "Main camera is absent!!!" );
	m_pActiveCamera->makeActive( gfx );
	RenderQueuePass::run( gfx );
}

void TransparentPass::setActiveCamera( const Camera &cam ) noexcept
{
	m_pActiveCamera = &cam;
}

}// ren