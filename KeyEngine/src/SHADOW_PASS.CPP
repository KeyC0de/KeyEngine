#include "shadow_pass.h"
#include "graphics.h"
#include "linker.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "depth_stencil_state.h"
#include "rasterizer_state.h"
#include "texture.h"
#include "cube_texture.h"
#include "camera.h"
#include "math_utils.h"
#include "texture_desc.h"
#include "texture_sampler_state.h"
#include "blend_state.h"
#include "settings_manager.h"
#include "global_constants.h"
#include "renderer_exception.h"
#if defined _DEBUG && !defined NDEBUG
#	include "d3d_utils.h"
#endif


namespace ren
{

unsigned ren::ShadowPass::getResolution() noexcept
{
	return s_shadowMapResolution;
}

ShadowPass::ShadowPass( Graphics &gfx,
	const std::string &name,
	const unsigned shadowMapRez )
	:
	RenderQueuePass{name},
	m_globalsVscb{gfx, s_globalsVscbSlot},
	m_globalsPscb{gfx, s_globalsPscbSlot},
	m_vscb{gfx, s_lightArrayVertexShaderCBSlot},
	m_pscb{gfx, s_lightArrayPixelShaderCBSlot}
{
	s_shadowMapResolution = shadowMapRez;

	addBindable( BlendState::fetch( gfx, BlendState::Mode::NoBlend, 0u ) );

	addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::ShadowTS, TextureSamplerState::FilterMode::Trilinear, TextureSamplerState::AddressMode::Border ) );
	addBindable( VertexShader::fetch( gfx, "shadow_vs.cso" ) );
	addBindable( PixelShaderNull::fetch( gfx ) );
	addBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::Default ) );

	addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::ShadowRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front, 50, 2.0f, 0.1f ) );

	addLinker( BindableLinker<TextureArrayOffscreenDS>::make( "offscreenShadowmapOut", m_pOffscreenDsvMapArray ) );
	addLinker( BindableLinker<CubeTextureArrayOffscreenDS>::make( "offscreenShadowCubemapOut", m_pOffscreenDsvCubemapArray ) );

	// create placeholder shadow maps for now
	m_pOffscreenDsvMapArray = std::make_shared<TextureArrayOffscreenDS>( gfx, s_shadowMapResolution, s_shadowMapResolution, s_shadowMapArraySlot, DepthStencilViewMode::ShadowDepth, 2u );
	m_pOffscreenDsvCubemapArray = std::make_shared<CubeTextureArrayOffscreenDS>( gfx, s_shadowMapResolution, s_shadowMapResolution, s_shadowCubeMapArraySlot, DepthStencilViewMode::ShadowDepth, 2u );

	std::vector<DirectX::XMFLOAT3> cameraDirections{6};
	std::vector<DirectX::XMFLOAT3> cameraUps{6};

	// +x
	DirectX::XMStoreFloat3( &cameraDirections[0], DirectX::XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f ) );
	m_cameraDirections[0] = DirectX::XMLoadFloat3( &cameraDirections[0] );
	DirectX::XMStoreFloat3( &cameraUps[0], DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	m_cameraUps[0] = DirectX::XMLoadFloat3( &cameraUps[0] );
	// -x
	DirectX::XMStoreFloat3( &cameraDirections[1], DirectX::XMVectorSet( -1.0f, 0.0f, 0.0f, 0.0f ) );
	m_cameraDirections[1] = DirectX::XMLoadFloat3( &cameraDirections[1] );
	DirectX::XMStoreFloat3( &cameraUps[1], DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	m_cameraUps[1] = DirectX::XMLoadFloat3( &cameraUps[1] );
	// +y
	DirectX::XMStoreFloat3( &cameraDirections[2], DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	m_cameraDirections[2] = DirectX::XMLoadFloat3( &cameraDirections[2] );
	DirectX::XMStoreFloat3( &cameraUps[2], DirectX::XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f ) );
	m_cameraUps[2] = DirectX::XMLoadFloat3( &cameraUps[2] );
	// -y
	DirectX::XMStoreFloat3( &cameraDirections[3], DirectX::XMVectorSet( 0.0f, -1.0f, 0.0f, 0.0f ) );
	m_cameraDirections[3] = DirectX::XMLoadFloat3( &cameraDirections[3] );
	DirectX::XMStoreFloat3( &cameraUps[3], DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ) );
	m_cameraUps[3] = DirectX::XMLoadFloat3( &cameraUps[3] );
	// +z
	DirectX::XMStoreFloat3( &cameraDirections[4], DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ) );
	m_cameraDirections[4] = DirectX::XMLoadFloat3( &cameraDirections[4] );
	DirectX::XMStoreFloat3( &cameraUps[4], DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	m_cameraUps[4] = DirectX::XMLoadFloat3( &cameraUps[4] );
	// -z
	DirectX::XMStoreFloat3( &cameraDirections[5], DirectX::XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f ) );
	m_cameraDirections[5] = DirectX::XMLoadFloat3( &cameraDirections[5] );
	DirectX::XMStoreFloat3( &cameraUps[5], DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
	m_cameraUps[5] = DirectX::XMLoadFloat3( &cameraUps[5] );
}

void ShadowPass::run( Graphics &gfx ) const cond_noex
{
	const_cast<ShadowPass*>( this )->bindGlobalCBs( gfx );

	//if ( util::modulus( gfx.getFrameNum(), g_nFramesPerShadowUpdate ) == 1 )	// #OPTIMIZATION: no need to update shadows every frame
	{
		const_cast<ShadowPass*>( this )->bindLightCBs( gfx );

		const unsigned nShadowCastingLights = m_shadowCastingLights.size();
		for ( int lightIndex = 0; lightIndex < nShadowCastingLights; ++lightIndex )
		{
			ILightSource *pLight = m_shadowCastingLights[lightIndex];

			gfx.setProjectionMatrix( pLight->getShadowCamera()->getProjectionMatrix(gfx, true, pLight->m_shadowCamFarZ) );

			const auto &pos = pLight->getPosition();
#if defined _DEBUG && !defined NDEBUG
			const auto &shadowCamPos = pLight->getShadowCamera()->getPosition();
			ASSERT( util::operator==(pos, shadowCamPos), "Inconsistent light and shadow camera positions!" );
#endif
			const auto posVec = DirectX::XMLoadFloat3( &pos );
			const LightSourceType lightType = pLight->getType();
			if ( lightType == LightSourceType::Directional )
			{
				const_cast<ShadowPass*>( this )->m_pDsv = m_pOffscreenDsvMapArray->shareDepthBuffer( lightIndex );
				m_pDsv->clear( gfx );
			}
			else if ( lightType == LightSourceType::Spot )
			{
				const_cast<ShadowPass*>( this )->m_pDsv = m_pOffscreenDsvMapArray->shareDepthBuffer( lightIndex );
				m_pDsv->clear( gfx );
			}
			else if ( lightType == LightSourceType::Point )
			{
				for ( int face = 0; face < 6; ++face )
				{
					// bind the DSV from the offscreen cube map ds texture
					const_cast<ShadowPass*>( this )->m_pDsv = m_pOffscreenDsvCubemapArray->shareDepthBuffer( lightIndex, face );
					m_pDsv->clear( gfx );

					const auto lookAt = DirectX::XMVectorAdd( posVec, m_cameraDirections[face] );
					gfx.setViewMatrix( DirectX::XMMatrixLookAtLH( posVec, lookAt, m_cameraUps[face] ) );
					RenderQueuePass::run( gfx );
				}
			}
		}
	}
}

void ShadowPass::bindShadowCastingLights( Graphics &gfx,
	const std::vector<ILightSource*> &shadowCastingLights )
{
	const unsigned nShadowCastingLights = shadowCastingLights.size();
#if defined _DEBUG && !defined NDEBUG
	{
		static const auto &settings = SettingsManager::getInstance().getSettings();
		if ( nShadowCastingLights > settings.iMaxShadowCastingDynamicLights )
		{
			THROW_RENDERER_EXCEPTION( "Attempted to create more shadow casting lights than the current limit of " + std::to_string( settings.iMaxShadowCastingDynamicLights ) + "!" );
		}
	}
#endif

	m_shadowCastingLights.clear();
	m_shadowCastingLights.reserve( nShadowCastingLights );

	for ( ILightSource *pLight : shadowCastingLights )
	{
		m_shadowCastingLights.push_back( pLight );
	}

	// create the Texture Arrays
	const unsigned nShadowCastingNonPointLights = std::count_if( m_shadowCastingLights.begin(), m_shadowCastingLights.end(), [] (const ILightSource *pLight) { return pLight->getType() != LightSourceType::Point; } );
	if ( nShadowCastingNonPointLights > 0 )
	{
		std::shared_ptr<TextureArrayOffscreenDS> temp_pOffscreenDsvMapArray = std::make_shared<TextureArrayOffscreenDS>( gfx, s_shadowMapResolution, s_shadowMapResolution, s_shadowMapArraySlot, DepthStencilViewMode::ShadowDepth, nShadowCastingNonPointLights );
		*m_pOffscreenDsvMapArray = *temp_pOffscreenDsvMapArray;
	}

	const unsigned nShadowCastingPointLights = std::count_if( m_shadowCastingLights.begin(), m_shadowCastingLights.end(), [] (const ILightSource *pLight) { return pLight->getType() == LightSourceType::Point; } );
	if ( nShadowCastingPointLights > 0 )
	{
		std::shared_ptr<CubeTextureArrayOffscreenDS> temp_pOffscreenDsvCubemapArray = std::make_shared<CubeTextureArrayOffscreenDS>( gfx, s_shadowMapResolution, s_shadowMapResolution, s_shadowCubeMapArraySlot, DepthStencilViewMode::ShadowDepth, nShadowCastingPointLights );
		*m_pOffscreenDsvCubemapArray = *temp_pOffscreenDsvCubemapArray;
	}

#if defined _DEBUG && !defined NDEBUG
	for ( unsigned lightIndex = 0; lightIndex < nShadowCastingNonPointLights; ++lightIndex )
	{
		m_pOffscreenDsvMapArray->accessDepthBuffer( lightIndex )->setDebugObjectName( std::string{"ShadowPassDsv#" + std::to_string( lightIndex )}.c_str() );
	}

	for ( unsigned lightIndex = 0; lightIndex < nShadowCastingPointLights; ++lightIndex )
	{
		for ( int face = 0; face < 6; ++face )
		{
			m_pOffscreenDsvCubemapArray->accessDepthBuffer( lightIndex, face )->setDebugObjectName( std::string{"ShadowPassDsv_cube#" + std::to_string( lightIndex ) + ":" + std::to_string( face )}.c_str() );
		}
	}
#endif
}

void ShadowPass::dumpShadowMap( Graphics &gfx,
	const std::string &path ) const
{
	auto pDsvShadowMap = m_pOffscreenDsvMapArray->shareDepthBuffer( 0 );
	pDsvShadowMap->convertToBitmap( gfx, s_shadowMapResolution, s_shadowMapResolution ).save( path +  "map_array_0.png" );
}

void ShadowPass::dumpShadowCubeMap( Graphics &gfx,
	const std::string &path ) const
{
	for ( size_t i = 0; i < 6; ++i )
	{
		auto pDsvShadowCubeMap = m_pOffscreenDsvCubemapArray->shareDepthBuffer( 0, i );
		pDsvShadowCubeMap->convertToBitmap( gfx, s_shadowMapResolution, s_shadowMapResolution ).save( path + "cube_map_array_0_" + std::to_string( i ) + ".png" );
	}
}

void ShadowPass::bindGlobalCBs( Graphics &gfx ) cond_noex
{
	const unsigned nShadowCastingLights = m_shadowCastingLights.size();
	const unsigned nShadowCastingPointLights = std::count_if( m_shadowCastingLights.begin(), m_shadowCastingLights.end(), [] (const ILightSource *pLight) { return pLight->getType() == LightSourceType::Point; } );

	const float seconds = gfx.getRealTimer().getSecondsFromStart();
	const GlobalsVSCB vscb{seconds, nShadowCastingLights, nShadowCastingPointLights, 0.0f};
	m_globalsVscb.update( gfx, vscb );
	m_globalsVscb.bind( gfx );

	const GlobalsPSCB pscb{seconds, nShadowCastingLights, nShadowCastingPointLights, 0.0f};
	m_globalsPscb.update( gfx, pscb );
	m_globalsPscb.bind( gfx );
}

void ShadowPass::bindLightCBs( Graphics &gfx ) cond_noex
{
	const unsigned nShadowCastingLights = m_shadowCastingLights.size();

	LightsVSCB vscbLightData{};
	LightsPSCB pscbLightData{};
	for ( int i = 0; i < nShadowCastingLights; ++i )
	{
		ILightSource *pLight = m_shadowCastingLights[i];
		pLight->populateCBData( gfx );
		vscbLightData[i] = pLight->getVscbData();
		pscbLightData[i] = pLight->getPscbData();
	}

	m_vscb.update( gfx, vscbLightData );
	m_vscb.bind( gfx );

	m_pscb.update( gfx, pscbLightData );
	m_pscb.bind( gfx );
}


}//namespace ren