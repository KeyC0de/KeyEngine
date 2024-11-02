#include "light_source.h"
#include "sphere.h"
#include "graphics.h"
#include "shadow_pass.h"
#include "camera.h"
#include "camera_manager.h"
#include "math_utils.h"
#include "d3d_utils.h"
#include "assertions_console.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
#endif


namespace dx = DirectX;

////////////////////////////////////////////////////////////////////////////////////////////////////
ILightSource::ILightSource( Graphics &gfx,
	const LightSourceType type,
	Model model,
	const std::variant<DirectX::XMFLOAT4,std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/,
	const DirectX::XMFLOAT3 &initialRotDeg /*= {0.0f, 0.0f, 0.f}*/,
	const DirectX::XMFLOAT3 &pos /*= {8.0f, 8.0f, 2.f}*/,
	const float intensity /*= 1.0f*/,
	const DirectX::XMFLOAT3 &direction /*= {0.0f, 0.0f, 0.0f}*/,
	const float fovDeg /*= 0.0f*/,
	const float shadowCamNearZ /*= 1.0f*/,
	const float shadowCamFarZ /*= 100.0f*/ )
	:
	m_type{type},
	m_bShowMesh{bShowMesh},
	m_bCastingShadows{bShadowCasting},
	m_rot{util::toRadians3( initialRotDeg )},
	m_lightMesh(std::move( model)),
	m_vscbData{},
	m_shadowCamFarZ{shadowCamFarZ}
{
	static int s_id = 0;
	++s_id;
	m_name = std::string{"Light#"} + std::to_string( s_id );

	const dx::XMFLOAT4 lightColor = std::holds_alternative<dx::XMFLOAT4>( colorOrTexturePath ) ? std::get<dx::XMFLOAT4>( colorOrTexturePath ) : dx::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f};
	m_pscbData = {static_cast<int>( type ),
		bShadowCasting ? 1 : 0,
		{-9.999, -9.999},
		pos,
		{0.08f, 0.08f, 0.08f},
		{lightColor.x, lightColor.y, lightColor.z},
		intensity,
		1.0f,
		0.025f,
		0.0030f,
		type == !LightSourceType::Point ? direction : dx::XMFLOAT3{0.0f,0.0f,0.0f},
		type == LightSourceType::Spot ? fovDeg : 0.0f};
	if ( bShadowCasting )
	{
		const DirectX::XMFLOAT3 &shadowCamPos = m_pscbData.cb_lightPosViewSpace;
		if ( type == LightSourceType::Directional )
		{
			ASSERT( util::operator==(shadowCamPos, dx::XMFLOAT3{0.0f, 0.0f, 0.0f}), "Directional Light position must is at the origin, very far away! (.w will be equal to 1 in shader)" );
			const std::pair<float, float> pitchAndYaw = Camera::computePitchYawInDegFromDirectionVector( direction );	// direction{0,0,0} = Directional Light direction vector looks at the center of the scene
			// ensure orthographic projection matrix is used to capture all relevant shadow details in the entire scene (with appropriately big farZ)
			// also make sure width and height are equal for Orthographic projection and big enough such that Directional/Star-light shadows cover pretty much the entire scene
			m_pCameraForShadowing = std::make_unique<Camera>( gfx, shadowCamFarZ, shadowCamFarZ, fovDeg, shadowCamPos, pitchAndYaw.first, pitchAndYaw.second, true, false, shadowCamNearZ, shadowCamFarZ );
		}
		else if ( type == LightSourceType::Spot )
		{
			const float shadowCamPitchDeg = initialRotDeg.x;
			const float shadowCamYawDeg = initialRotDeg.y;

			// set 1.0 aspect ratio for a Spot Light to have a square frustum; the spotlight is typically symmetric (meaning it has the same horizontal and vertical FOV)
			m_pCameraForShadowing = std::make_unique<Camera>( gfx, shadowCamFarZ, shadowCamFarZ, fovDeg, shadowCamPos, shadowCamPitchDeg, shadowCamYawDeg, true, true, shadowCamNearZ, shadowCamFarZ );
		}
		else if ( type == LightSourceType::Point )
		{
			const float shadowCamPitchDeg = initialRotDeg.x;
			const float shadowCamYawDeg = initialRotDeg.y;

			m_pCameraForShadowing = std::make_unique<Camera>( gfx, gfx.getClientWidth(), gfx.getClientHeight(), fovDeg, shadowCamPos, shadowCamPitchDeg, shadowCamYawDeg, true, true, shadowCamNearZ, shadowCamFarZ );
		}

	}
}

void ILightSource::update( Graphics &gfx,
	const float dt,
	const float lerpBetweenFrames,
	const bool bEnableSmoothMovement /*= false*/ ) cond_noex
{
	if ( m_bShowMesh )
	{
		m_lightMesh.update( dt, lerpBetweenFrames, bEnableSmoothMovement );
	}
}

void ILightSource::render( const size_t channels ) const cond_noex
{
	if ( m_bShowMesh )
	{
		m_lightMesh.render( channels );
	}
}

void ILightSource::populateCBData( Graphics &gfx ) cond_noex
{
	populateVscbData( gfx );
	populatePscbData( gfx );
}

void ILightSource::connectMaterialsToRenderer( ren::Renderer &r )
{
	m_lightMesh.connectMaterialsToRenderer( r );
}

void ILightSource::setRotation( const DirectX::XMFLOAT3 &rot )
{
	if ( m_bCastingShadows )
	{
		m_pCameraForShadowing->setRotation( rot );
	}
	m_lightMesh.setRotation( rot );
}

void ILightSource::setTranslation( const DirectX::XMFLOAT3 &pos )
{
	if ( m_bCastingShadows )
	{
		m_pCameraForShadowing->setTethered( false );
		m_pCameraForShadowing->setTranslation( pos );
		m_pCameraForShadowing->setTethered( true );
	}
	m_lightMesh.setTranslation( pos );
}

LightSourceType ILightSource::getType() const noexcept
{
	return m_type;
}

unsigned ILightSource::getSlot() const noexcept
{
	return static_cast<unsigned>( m_pscbData.cb_lightType );
}

std::string ILightSource::getName() const noexcept
{
	return m_name;
}

bool ILightSource::isCastingShadows() const noexcept
{
	return m_bCastingShadows;
}

// #TODO:
bool ILightSource::isFrustumCulled() const noexcept
{
	return false;
}

DirectX::XMFLOAT3 ILightSource::getRotation() const noexcept
{
	return m_lightMesh.getRotation();
}

Camera* ILightSource::getShadowCamera() const
{
	ASSERT( m_bCastingShadows, "There is no shadow camera!" );
	return m_pCameraForShadowing.get();
}

float ILightSource::getShadowCameraFarZ() const noexcept
{
	return m_shadowCamFarZ;
}

ILightSource::LightVSCB ILightSource::getVscbData() noexcept
{
	return m_vscbData;
}

ILightSource::LightPSCB ILightSource::getPscbData() noexcept
{
	return m_pscbDataPrev;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
DirectionalLight::DirectionalLight( Graphics &gfx,
	const float radiusScale /*= 1.0f*/,
	const DirectX::XMFLOAT3 &initialRotDeg /*= {0.0f, 0.0f, 0.f}*/,
	const DirectX::XMFLOAT3 &pos /*= {8.0f, 8.0f, 2.f}*/,
	const std::variant<DirectX::XMFLOAT4,std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/,
	const float intensity /*= 1.0f*/,
	const DirectX::XMFLOAT3 &direction /*= {0.0f, 0.0f, 0.0f}*/,
	const float shadowCamNearZ /*= 1.0f*/,
	const float shadowCamFarZ /*= 2000.0f*/ )
	:
	ILightSource(gfx, LightSourceType::Directional, Model(std::make_unique<Sphere>(gfx, radiusScale, colorOrTexturePath), gfx, initialRotDeg, pos), colorOrTexturePath, bShadowCasting, bShowMesh, initialRotDeg, pos, intensity, direction, shadowCamNearZ, shadowCamFarZ)
{

}

void DirectionalLight::displayImguiWidgets() noexcept
{
	pass_;
}

void DirectionalLight::setIntensity( const float newIntensity ) noexcept
{
	m_pscbData.intensity = newIntensity;
}

void DirectionalLight::setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept
{
	m_pscbData.lightColor = diffuseColor;
}

DirectX::XMFLOAT3 DirectionalLight::getPosition() const noexcept
{
	return m_lightMesh.getPosition();
}

void DirectionalLight::populateVscbData( Graphics &gfx ) cond_noex
{
	ASSERT( m_pCameraForShadowing, "Camera not specified (null)!" );
	m_vscbData = {dx::XMMatrixTranspose( m_pCameraForShadowing->getViewMatrix() * m_pCameraForShadowing->getProjectionMatrix( gfx, true, m_shadowCamFarZ ) )};
}

void DirectionalLight::populatePscbData( Graphics &gfx ) cond_noex
{
	/*
	ASSERT( m_pDirectionalLightShadowCamera, "Camera not specified (null)!" );
	dx::XMFLOAT3 dir;
	dx::XMStoreFloat3( &dir, m_pDirectionalLightShadowCamera->getDirection() );
	const DirectionalPointLightSourceShadowTransformVSCB vscb{dx::XMMatrixTranspose( dx::XMMatrixTranslation( -dir.x, -dir.y, -dir.z ) )};
	m_pVscb->update( gfx, vscb );
	*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SpotLight::SpotLight( Graphics &gfx,
	const float radiusScale /*= 1.0f*/,
	const DirectX::XMFLOAT3 &initialRotDeg /*= {0.0f, 0.0f, 0.f}*/,
	const DirectX::XMFLOAT3 &pos /*= {8.0f, 8.0f, 2.f}*/,
	const std::variant<DirectX::XMFLOAT4,std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/,
	const float intensity /*= 1.0f*/,
	const DirectX::XMFLOAT3 &direction /*= {0.0f, 0.0f, 1.0f}*/,
	const float fovConeAngle /*= 60.0f*/,
	const float shadowCamNearZ /*= 0.5f*/,
	const float shadowCamFarZ /*= 80.0f*/ )
	:
	ILightSource(gfx, LightSourceType::Spot, Model(std::make_unique<Sphere>(gfx, radiusScale, colorOrTexturePath), gfx, initialRotDeg, pos), colorOrTexturePath, bShadowCasting, bShowMesh, initialRotDeg, pos, intensity, direction, fovConeAngle, shadowCamNearZ, shadowCamFarZ)
{

}

void SpotLight::displayImguiWidgets() noexcept
{
	pass_;
}

void SpotLight::setIntensity( const float newIntensity ) noexcept
{
	m_pscbData.intensity = newIntensity;
}

void SpotLight::setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept
{
	m_pscbData.lightColor = diffuseColor;
}

DirectX::XMFLOAT3 SpotLight::getPosition() const noexcept
{
	return m_pscbData.cb_lightPosViewSpace;
}

void SpotLight::populateVscbData( Graphics &gfx ) cond_noex
{
	ASSERT( m_pCameraForShadowing, "Camera not specified (null)!" );
	m_vscbData = {dx::XMMatrixTranspose( m_pCameraForShadowing->getViewMatrix() * m_pCameraForShadowing->getProjectionMatrix( gfx, true, m_shadowCamFarZ ) )};
}

void SpotLight::populatePscbData( Graphics &gfx ) cond_noex
{
	pass_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
PointLight::PointLight( Graphics &gfx,
	const float radiusScale /*= 0.5f*/,
	const DirectX::XMFLOAT3 &initialRotDeg /*= {0.0f, 0.0f, 0.f}*/,
	const DirectX::XMFLOAT3 &pos /*= {8.0f, 8.0f, 2.f}*/,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/,
	const float intensity /*= 1.0f*/,
	const float shadowCamNearZ /*= 1.0f*/,
	const float shadowCamFarZ /*= 100.0f*/ )
	:
	ILightSource(gfx, LightSourceType::Point, Model(std::make_unique<Sphere>(gfx, radiusScale, colorOrTexturePath), gfx, initialRotDeg, pos), colorOrTexturePath, bShadowCasting, bShowMesh, initialRotDeg, pos, intensity, {0.0f, 0.0f, 0.0f}, 90.0f, shadowCamNearZ, shadowCamFarZ)
{
	
}

void PointLight::displayImguiWidgets() noexcept
{
#ifndef FINAL_RELEASE
	if ( ImGui::Begin( m_name.c_str() ) )
	{
		// #TODO: remove rotation from Point Light
		bool bDirtyRot = false;
		const auto dirtyCheckRot = [&bDirtyRot]( const bool bChanged )
		{
			bDirtyRot = bDirtyRot || bChanged;
		};

		ImGui::Text( "Rotation" );
		dirtyCheckRot( ImGui::SliderFloat( "Pitch", &m_rot.x, -60.0f, 60.0f, "%.1f" ) );
		dirtyCheckRot( ImGui::SliderFloat( "Yaw", &m_rot.y, -60.0f, 60.0f, "%.1f" ) );
		dirtyCheckRot( ImGui::SliderFloat( "Roll", &m_rot.z, -60.0f, 60.0f, "%.1f" ) );

		if ( bDirtyRot )
		{
			setRotation( m_rot );
		}

		bool bDirtyPos = false;
		const auto dirtyCheckPos = [&bDirtyPos]( const bool bChanged )
		{
			bDirtyPos = bDirtyPos || bChanged;
		};

		ImGui::Text( "Position" );
		dirtyCheckPos( ImGui::SliderFloat( "X", &m_pscbData.cb_lightPosViewSpace.x, -100.0f, 100.0f, "%.1f" ) );
		dirtyCheckPos( ImGui::SliderFloat( "Y", &m_pscbData.cb_lightPosViewSpace.y, -100.0f, 100.0f, "%.1f" ) );
		dirtyCheckPos( ImGui::SliderFloat( "Z", &m_pscbData.cb_lightPosViewSpace.z, -100.0f, 100.0f, "%.1f" ) );

		if ( bDirtyPos )
		{
			setTranslation( m_pscbData.cb_lightPosViewSpace );
		}

		ImGui::Text( "Intensity & Color" );
		ImGui::SliderFloat( "Intensity", &m_pscbData.intensity, 0.01f, 4.0f, "%.2f", 2.0f );
		ImGui::ColorEdit3( "Diffuse", &m_pscbData.lightColor.x );	// #TODO: expose model's material properties cb to change its color when changing the light's color here
		ImGui::ColorEdit3( "Ambient", &m_pscbData.ambient.x );

		ImGui::Text( "Attenuation" );
		ImGui::SliderFloat( "Constant", &m_pscbData.attConstant, 0.05f, 10.0f, "%.2f", 4.0f );
		ImGui::SliderFloat( "Linear", &m_pscbData.attLinear, 0.0001f, 4.0f, "%.4f", 8.0f );
		ImGui::SliderFloat( "Quadratic", &m_pscbData.attQuadratic, 0.0000001f, 10.0f, "%.7f", 10.0f);

		ImGui::Checkbox( "Show Sphere Mesh", &m_bShowMesh );
	}
	ImGui::End();
#endif
}

void PointLight::setIntensity( const float newIntensity ) noexcept
{
	m_pscbData.intensity = newIntensity;
}

void PointLight::setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept
{
	m_pscbData.lightColor = diffuseColor;
}

DirectX::XMFLOAT3 PointLight::getPosition() const noexcept
{
	return m_pscbData.cb_lightPosViewSpace;
}

void PointLight::populateVscbData( Graphics &gfx ) cond_noex
{
	ASSERT( m_pCameraForShadowing, "Camera not specified (null)!" );
	const auto &pos = m_pCameraForShadowing->getPosition();
	m_vscbData = {dx::XMMatrixTranspose( dx::XMMatrixTranslation( -pos.x, -pos.y, -pos.z ) )};
}

void PointLight::populatePscbData( Graphics &gfx ) cond_noex
{
	static CameraManager &s_cameraMan = CameraManager::getInstance();
	const auto &activeCameraViewMat = s_cameraMan.getActiveCamera().getViewMatrix();
	m_pscbDataPrev = m_pscbData;
	const auto lightPos = DirectX::XMLoadFloat3( &m_pscbData.cb_lightPosViewSpace );
	DirectX::XMStoreFloat3( &m_pscbDataPrev.cb_lightPosViewSpace, DirectX::XMVector3Transform( lightPos, activeCameraViewMat ) );
}