#include "transparent_pass.h"
#include "camera.h"
#include "pixel_shader.h"
#include "vertex_shader.h"
#include "depth_stencil_state.h"
#include "linker.h"
#include "render_target_view.h"


namespace ren
{

TransparentPass::TransparentPass( Graphics &gfx,
	const std::string &name )
	:
	RenderQueuePass{name, {}, true}
{
	addBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::DepthReadOnlyStencilOff ) );

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