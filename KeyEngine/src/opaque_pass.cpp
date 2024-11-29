#include "opaque_pass.h"
#include <string>
#include "camera.h"
#include "binder.h"
#include "linker.h"
#include "render_target_view.h"
#include "depth_stencil_view.h"
#include "depth_stencil_state.h"
#include "assertions_console.h"
#include "texture.h"
#include "cube_texture.h"


namespace ren
{

OpaquePass::OpaquePass( Graphics &gfx,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::Default ) );

	// TextureSamplerState is set per mesh depending on whether the mesh has a texture

	// RasterizerState is set per object depending on whether the object's texture has an alpha channel

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
	addBindableBinderUsingContainerIndex<TextureArrayOffscreenDS>( "offscreenShadowmapIn" );
	addBindableBinderUsingContainerIndex<CubeTextureArrayOffscreenDS>( "offscreenShadowCubemapIn" );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
}

void OpaquePass::run( Graphics &gfx ) const cond_noex
{
	ASSERT( m_pActiveCamera, "Main camera is absent!!!" );
	m_pActiveCamera->makeActive( gfx );
	RenderQueuePass::run( gfx );
}

void OpaquePass::setActiveCamera( const Camera &cam ) noexcept
{
	m_pActiveCamera = &cam;
}

}//ren