#include "shadow_pass.h"
#include "graphics.h"
#include "linker.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "depth_stencil_view.h"
#include "depth_stencil_state.h"
#include "blend_state.h"
#include "rasterizer_state_shadow.h"
#include "cube_texture.h"
#include "camera.h"
#include "math_utils.h"
#include "light_vscb.h"
#include "texture_desc.h"
#include "shadow_map_sampler_state.h"


namespace ren
{

ShadowPass::ShadowPass( Graphics &gph,
	const std::string &name,
	const unsigned shadowMapRez )
	:
	RenderQueuePass{name},
	m_pLightVcb{std::make_shared<LightVSCB>( gph, 1u )}
{
	s_shadowMapResolution = shadowMapRez;

	addPassBindable( m_pLightVcb );
	addPassBindable( std::make_shared<ShadowMapSamplerState>( gph,
		true,
		ShadowMapSamplerState::FilterMode::Trilinear ) );
	addPassBindable( VertexShader::fetch( gph,
		"shadow_vs.cso" ) );
	addPassBindable( PixelShaderNull::fetch( gph ) );
	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::Default ) );

	addPassBindable( std::make_shared<RasterizerStateShadow>( gph,
		50,
		2.0f,
		0.1f ) );

	addPassBindable( BlendState::fetch( gph,	// #TODO: you could remove this
		BlendState::NoBlend,
		3u ) );

	// create the offscreen texture
	m_pOffscreenDsvCubemap = std::make_shared<CubeTextureOffscreenDS>( gph,
		s_shadowMapResolution,
		s_shadowMapResolution,
		3u,
		DepthStencilViewMode::ShadowDepth );

#if defined _DEBUG && !defined NDEBUG
	for ( size_t i = 0; i < 6; ++i )
	{
		m_pOffscreenDsvCubemap->depthBuffer( i )->setDebugObjectName( std::string{"ShadowPassDsv#" + std::to_string( i )}.c_str() );
	}
#endif

	addLinker( BindableLinker<CubeTextureOffscreenDS>::make( "offscreenShadowCubemapOut",
		m_pOffscreenDsvCubemap ) );

	// bind the DSV from the offscreen cube map ds texture side #0
	m_pDsv = m_pOffscreenDsvCubemap->shareDepthBuffer( 0u );

	DirectX::XMStoreFloat4x4( &m_cameraShadowProjectionMatrix,
			Camera::getShadowProjectionMatrix() );

	// +x
	DirectX::XMStoreFloat3( &m_cameraDirections[0],
		DirectX::XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[0],
		DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	// -x
	DirectX::XMStoreFloat3( &m_cameraDirections[1],
		DirectX::XMVectorSet( -1.0f, 0.0f, 0.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[1],
		DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	// +y
	DirectX::XMStoreFloat3( &m_cameraDirections[2],
		DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[2],
		DirectX::XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f ) );
	// -y
	DirectX::XMStoreFloat3( &m_cameraDirections[3],
		DirectX::XMVectorSet( 0.0f, -1.0f, 0.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[3],
		DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ) );
	// +z
	DirectX::XMStoreFloat3( &m_cameraDirections[4],
		DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[4],
		DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	// -z
	DirectX::XMStoreFloat3( &m_cameraDirections[5],
		DirectX::XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[5],
		DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
}

void ShadowPass::run( Graphics &gph ) const cond_noex
{
	m_pLightVcb->update( gph );
	const auto pos = DirectX::XMLoadFloat3( &m_pShadowCamera->getPosition() );

	gph.setProjectionMatrix( DirectX::XMLoadFloat4x4( &m_cameraShadowProjectionMatrix ) );
	for ( size_t i = 0; i < 6; ++i )
	{
		const_cast<ShadowPass*>( this )->m_pDsv = m_pOffscreenDsvCubemap->shareDepthBuffer( i );
		m_pDsv->clear( gph );

		const auto lookAt = DirectX::XMVectorAdd( pos,
			DirectX::XMLoadFloat3( &m_cameraDirections[i] ) );
		gph.setViewMatrix( DirectX::XMMatrixLookAtLH( pos,
			lookAt,
			DirectX::XMLoadFloat3( &m_cameraUps[i] ) ) );
		RenderQueuePass::run( gph );
	}
}

void ShadowPass::setShadowCamera( const Camera &cam ) noexcept
{
	m_pLightVcb->setCamera( &cam );
	m_pShadowCamera = &cam;
}

void ShadowPass::dumpShadowMap( Graphics &gph,
	const std::string &path ) const
{
	for ( size_t i = 0; i < 6; ++i )
	{
		auto pDsvCubeTex = m_pOffscreenDsvCubemap->shareDepthBuffer( i );
		pDsvCubeTex->convertToBitmap( gph,
			gph.getClientWidth(),
			gph.getClientHeight() ).save( path + std::to_string( i ) + ".png" );
	}
}

const unsigned ren::ShadowPass::getResolution() noexcept
{
	return s_shadowMapResolution;
}


}//ren