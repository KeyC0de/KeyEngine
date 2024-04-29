#include "shadow_pass.h"
#include "primitive_topology.h"
#include "graphics.h"
#include "linker.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "depth_stencil_view.h"
#include "depth_stencil_state.h"
#include "blend_state.h"
#include "rasterizer_state.h"
#include "cube_texture.h"
#include "camera.h"
#include "math_utils.h"
#include "light_source_shadow_vscb.h"
#include "texture_desc.h"
#include "texture_sampler_state.h"


namespace ren
{

ShadowPass::ShadowPass( Graphics &gph,
	const std::string &name,
	const unsigned shadowMapRez )
	:
	RenderQueuePass{name},
	m_pLightVcb{std::make_shared<PointLightSourceShadowVSCB>( gph, 1u )}		// #TODO: currently supporting 1 shadow casting light source, for multiple ones (gotta have an upper limit too) supply the right slot to PointLightSourceShadowVSCB and also iterate through them appropriately below, @ ShadowPass::run, @ ShadowPass::setShadowCamera(cam, bAddOrRemove), also careful which slot # you pass here (must agree with PointLightSourceShadowTransformVSCB : register(b1) in shadowing_vs.hlsli)
{
	s_shadowMapResolution = shadowMapRez;

	addPassBindable( PrimitiveTopology::fetch( gph, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	addPassBindable( m_pLightVcb );
	addPassBindable( TextureSamplerState::fetch( gph, TextureSamplerState::TextureSamplerMode::ShadowTS, TextureSamplerState::FilterMode::Trilinear, TextureSamplerState::AddressMode::Border ) );
	addPassBindable( VertexShader::fetch( gph, "shadow_vs.cso" ) );
	addPassBindable( PixelShaderNull::fetch( gph ) );
	addPassBindable( DepthStencilState::fetch( gph, DepthStencilState::Mode::Default ) );

	addPassBindable( RasterizerState::fetch( gph, RasterizerState::RasterizerMode::ShadowRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front, 50, 2.0f, 0.1f ) );

	// create the offscreen texture
	m_pOffscreenDsvCubemap = std::make_shared<CubeTextureOffscreenDS>( gph, s_shadowMapResolution, s_shadowMapResolution, 3u, DepthStencilViewMode::ShadowDepth );

#if defined _DEBUG && !defined NDEBUG
	for ( size_t i = 0; i < 6; ++i )
	{
		m_pOffscreenDsvCubemap->depthBuffer( i )->setDebugObjectName( std::string{"ShadowPassDsv#" + std::to_string( i )}.c_str() );	// #TODO: prepend to the name the id of the light
	}
#endif

	addLinker( BindableLinker<CubeTextureOffscreenDS>::make( "offscreenShadowCubemapOut", m_pOffscreenDsvCubemap ) );

	// bind the DSV from the offscreen cube map ds texture side #0
	m_pDsv = m_pOffscreenDsvCubemap->shareDepthBuffer( 0u );

	DirectX::XMStoreFloat4x4( &m_cameraShadowProjectionMatrix, Camera::getShadowProjectionMatrix() );

	// +x
	DirectX::XMStoreFloat3( &m_cameraDirections[0], DirectX::XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[0], DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	// -x
	DirectX::XMStoreFloat3( &m_cameraDirections[1], DirectX::XMVectorSet( -1.0f, 0.0f, 0.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[1], DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	// +y
	DirectX::XMStoreFloat3( &m_cameraDirections[2], DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[2], DirectX::XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f ) );
	// -y
	DirectX::XMStoreFloat3( &m_cameraDirections[3], DirectX::XMVectorSet( 0.0f, -1.0f, 0.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[3], DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ) );
	// +z
	DirectX::XMStoreFloat3( &m_cameraDirections[4], DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[4], DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	// -z
	DirectX::XMStoreFloat3( &m_cameraDirections[5], DirectX::XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f ) );
	DirectX::XMStoreFloat3( &m_cameraUps[5], DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
}

void ShadowPass::run( Graphics &gph ) const cond_noex
{
	if ( m_pLightVcb->isCastingShadows() )
	{
		m_pLightVcb->update( gph );
		const auto pos = DirectX::XMLoadFloat3( &m_pCameraForShadowing->getPosition() );

		gph.setProjectionMatrix( DirectX::XMLoadFloat4x4( &m_cameraShadowProjectionMatrix ) );
		for ( size_t i = 0; i < 6; ++i )
		{
			const_cast<ShadowPass*>( this )->m_pDsv = m_pOffscreenDsvCubemap->shareDepthBuffer( i );
			m_pDsv->clear( gph );
	
			const auto lookAt = DirectX::XMVectorAdd( pos, DirectX::XMLoadFloat3( &m_cameraDirections[i] ) );
			gph.setViewMatrix( DirectX::XMMatrixLookAtLH( pos, lookAt, DirectX::XMLoadFloat3( &m_cameraUps[i] ) ) );
			RenderQueuePass::run( gph );
		}
	}
}

void ShadowPass::setShadowCamera( const Camera &cam,
	const bool bEnable ) noexcept
{
	m_pLightVcb->setCamera( &cam, bEnable );
	if ( bEnable )
	{
		m_pCameraForShadowing = &cam;
	}
}

void ShadowPass::dumpShadowMap( Graphics &gph,
	const std::string &path ) const
{
	for ( size_t i = 0; i < 6; ++i )
	{
		auto pDsvCubeTex = m_pOffscreenDsvCubemap->shareDepthBuffer( i );
		pDsvCubeTex->convertToBitmap( gph, gph.getClientWidth(), gph.getClientHeight() ).save( path + std::to_string( i ) + ".png" );
	}
}

unsigned ren::ShadowPass::getResolution() noexcept
{
	return s_shadowMapResolution;
}


}//ren