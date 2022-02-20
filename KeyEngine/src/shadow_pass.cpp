#include "shadow_pass.h"
#include "graphics.h"
#include "producer.h"
#include "vertex_shader.h"
#include "pixel_shader_null.h"
#include "depth_stencil_view.h"
#include "depth_stencil_state.h"
#include "blend_state.h"
#include "rasterizer_shadow.h"
#include "cube_texture.h"
#include "camera.h"
#include "math_utils.h"
#include "light_vcb.h"
#include "shadow_map_sampler.h"


namespace ren
{

ShadowPass::ShadowPass( Graphics& gph,
	const std::string& name,
	unsigned shadowMapRez )
	:
	RenderQueuePass{name},
	m_pLightVcb{std::make_shared<LightVCB>( gph, 1u )}
{
	m_shadowMapResolution = shadowMapRez;

	addPassBindable( m_pLightVcb );
	addPassBindable( std::make_shared<ShadowMapSampler>( gph ) );
	m_pDsvCubemap = std::make_shared<CubeTextureDS>( gph,
		m_shadowMapResolution,
		3u );
	addPassBindable( VertexShader::fetch( gph,
		"shadow_vs.cso" ) );
	addPassBindable( PixelShaderNull::fetch( gph ) );
	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::Default ) );

	addPassBindable( std::make_shared<RasterizerShadow>( gph,
		50,
		2.0f,
		0.1f ) );
	addProducer( BindableProducer<CubeTextureDS>::make( "shadowCubemapRttOut",
		m_pDsvCubemap ) );

	addPassBindable( BlendState::fetch( gph,
		BlendState::NoBlend,
		3u ) );

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

	// get the depth stencil view
	setDepthStencilView( m_pDsvCubemap->shareDepthBuffer( 0 ) );
}

void ShadowPass::run( Graphics& gph ) const cond_noex
{
	m_pLightVcb->update( gph );
	const auto pos = DirectX::XMLoadFloat3( &m_pShadowCamera->getPosition() );

	gph.setProjectionMatrix( DirectX::XMLoadFloat4x4( &m_cameraShadowProjectionMatrix ) );
	for ( size_t i = 0; i < 6; ++i )
	{
		auto dsv = m_pDsvCubemap->shareDepthBuffer( i );
		dsv->clear( gph );
		setDepthStencilView( std::move( dsv ) );

		const auto lookAt = DirectX::XMVectorAdd( pos,
			DirectX::XMLoadFloat3( &m_cameraDirections[i] ) );
		gph.setViewMatrix( DirectX::XMMatrixLookAtLH( pos,
			lookAt,
			DirectX::XMLoadFloat3( &m_cameraUps[i] ) ) );
		RenderQueuePass::run( gph );
	}
}

void ShadowPass::setShadowCamera( const Camera& cam ) noexcept
{
	m_pLightVcb->setCamera( &cam );
	m_pShadowCamera = &cam;
}

void ShadowPass::dumpShadowMap( Graphics& gph,
	const std::string& path ) const
{
	for ( size_t i = 0; i < 6; ++i )
	{
		auto pDsvCubeTex = m_pDsvCubemap->shareDepthBuffer( i );
		pDsvCubeTex->convertToBitmap( gph ).save( path + std::to_string( i ) + ".png" );
	}
}

unsigned ren::ShadowPass::getResolution() noexcept
{
	return m_shadowMapResolution;
}

void ShadowPass::setDepthStencilView( std::shared_ptr<IDepthStencilView> ds ) const
{
	const_cast<ShadowPass*>( this )->m_pDsv = std::move( ds );
}


}//ren