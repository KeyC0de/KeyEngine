#include "lambertian_pass.h"
#include <string>
#include "primitive_topology.h"
#include "binder.h"
#include "linker.h"
#include "camera.h"
#include "render_target.h"
#include "depth_stencil_view.h"
#include "depth_stencil_state.h"
#include "texture_sampler_state.h"
#include "assertions_console.h"
#include "cube_texture.h"


namespace ren
{

LambertianPass::LambertianPass( Graphics &gfx,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addPassBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	addPassBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

	addPassBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::Default ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
	addContainerBindableBinder<CubeTextureOffscreenDS>( "offscreenShadowCubemapIn" );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
}

void LambertianPass::setActiveCamera( const Camera &cam ) noexcept
{
	m_pActiveCamera = &cam;
}

void LambertianPass::run( Graphics &gfx ) const cond_noex
{
	ASSERT( m_pActiveCamera, "Main camera is absent!!!" );
	m_pActiveCamera->makeActive( gfx, false );
	RenderQueuePass::run( gfx );
}


}//ren