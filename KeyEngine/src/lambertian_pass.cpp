#include "lambertian_pass.h"
#include <string>
#include "consumer.h"
#include "producer.h"
#include "camera.h"
#include "render_target.h"
#include "depth_stencil_view.h"
#include "depth_stencil_state.h"
#include "texture_sampler_state.h"
#include "assertions_console.h"
#include "cube_texture.h"


namespace ren
{

LambertianPass::LambertianPass( Graphics &gph,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addPassBindable( std::make_shared<TextureSamplerState>( gph,
		0u,
		TextureSamplerState::FilterMode::Anisotropic,
		TextureSamplerState::AddressMode::Wrap ) );

	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::Default ) );

	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
	addContainerBindableConsumer<CubeTextureDS>( "offscreenShadowCubemapIn" );

	addProducer( RenderSurfaceProducer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addProducer( RenderSurfaceProducer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
}

void LambertianPass::setActiveCamera( const Camera &cam ) noexcept
{
	m_pActiveCamera = &cam;
}

void LambertianPass::run( Graphics &gph ) const cond_noex
{
	ASSERT( m_pActiveCamera, "Main camera is absent!!!" );
	m_pActiveCamera->makeActive( gph,
		false );
	RenderQueuePass::run( gph );
}


}//ren