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

constexpr float g_directionalLightFovDeg = 0.0f;	// irrelevant for directional light shadows since they use Orthographic Projection, so we pass 0.0f (in reality the fov is ~ 90.0f)
constexpr float g_pointLightFovDeg = 90.0f;			// must be 90 for Point Light Cube mapping to work

////////////////////////////////////////////////////////////////////////////////////////////////////
ILightSource::ILightSource( Graphics &gfx,
	const LightSourceType type,
	Model model,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/,
	const DirectX::XMFLOAT3 &rotDegOrDirectionVector /*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT3 &pos /*= {8.0f, 8.0f, 2.f}*/,
	const float intensity /*= 1.0f*/,
	const float fovDeg /*= 0.0f*/,
	const float shadowCamFarZ /*= 100.0f*/ )
	:
	m_type{type},
	m_bShowMesh{bShowMesh},
	m_bShadowCasting{bShadowCasting},
	m_lightMesh(std::move( model)),
	m_vscbData{},
	m_shadowCamFarZ{shadowCamFarZ}
{
	static int s_id = 0;
	++s_id;
	m_name = std::string{"Light#"} + std::to_string( s_id );

	const DirectX::XMFLOAT3 directionVec = util::toRadians3( rotDegOrDirectionVector );

	const dx::XMFLOAT4 lightColor = std::holds_alternative<dx::XMFLOAT4>( colorOrTexturePath ) ? std::get<dx::XMFLOAT4>( colorOrTexturePath ) : dx::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f};
	m_pscbData = {static_cast<int>( type ),
		bShadowCasting ? 1 : 0,
		{-9.999, -9.999},
		type == LightSourceType::Directional ? directionVec : pos,
		{lightColor.x, lightColor.y, lightColor.z},
		intensity,
		type == LightSourceType::Directional ? 0.0f : 1.0f,		// no attenuation for directional lights
		type == LightSourceType::Directional ? 0.0f : 0.025f,
		type == LightSourceType::Directional ? 0.0f : 0.0030f,
		type == LightSourceType::Spot ? directionVec : dx::XMFLOAT3{0.0f,0.0f,0.0f},
		type == LightSourceType::Spot ? fovDeg : 0.0f};
	m_pscbDataToBind = m_pscbData;
}

void ILightSource::update( const float dt,
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
	ASSERT( m_type != LightSourceType::Point, "You shouldn't need to change the rotation of Point Lights" );
	m_pscbData.cb_lightPosViewSpace = rot;
	if ( m_bShadowCasting )
	{
		m_pCameraForShadowing->setTethered( false );
		m_pCameraForShadowing->setRotation( rot );
		m_pCameraForShadowing->setTethered( true );
	}
	m_lightMesh.setRotation( rot );
}

void ILightSource::setTranslation( const DirectX::XMFLOAT3 &pos )
{
	ASSERT( m_type != LightSourceType::Directional, "You shouldn't need to change the position of Directional Lights" );
	m_pscbData.cb_lightPosViewSpace = pos;
	if ( m_bShadowCasting )
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
	return m_bShadowCasting;
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
	ASSERT( m_bShadowCasting, "There is no shadow camera!" );
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
	return m_pscbDataToBind;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
DirectionalLight::DirectionalLight( Graphics &gfx,
	const float radiusScale /*= 1.0f*/,
	const DirectX::XMFLOAT3 &directionVector /*= {0.0f, 0.0f, 0.0f}*/,	// direction{0,0,0} = Directional Light's direction vector looks at the center of the scene
	const DirectX::XMFLOAT3 &lightMeshPos /*= {8.0f, 8.0f, 2.f}*/,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/,
	const float intensity /*= 1.0f*/,
	const float shadowCamNearZ /*= 1.0f*/,
	const float shadowCamFarZ /*= 2000.0f*/ )
	:
	ILightSource(gfx, LightSourceType::Directional, Model(std::make_unique<Sphere>(gfx, radiusScale, colorOrTexturePath), gfx, Camera::computePitchYawRollInDegFromDirectionVector(directionVector), lightMeshPos), colorOrTexturePath, bShadowCasting, bShowMesh, directionVector, lightMeshPos, intensity, 0.0f, shadowCamFarZ)
{
	if ( bShadowCasting )
	{
		ASSERT( util::operator==(m_pscbData.cb_lightPosViewSpace /*=== directionVector, != lightMeshPos*/, dx::XMFLOAT3{0.0f, 0.0f, 0.0f}), "Directional Light direction must be at the origin! (.w will be equal to 1 in shader)" );
		const std::pair<float, float> pitchAndYaw = Camera::computePitchYawInDegFromDirectionVector( directionVector );
		// ensure orthographic projection matrix is used to capture all relevant shadow details in the entire scene (with appropriately big farZ)
		// also make sure width and height are equal for Orthographic projection and big enough such that Directional/Star-light shadows cover pretty much the entire scene
		m_pCameraForShadowing = std::make_unique<Camera>( gfx, shadowCamFarZ, shadowCamFarZ, g_directionalLightFovDeg, lightMeshPos, pitchAndYaw.first, pitchAndYaw.second, true, false, shadowCamNearZ, shadowCamFarZ );
	}
}

void DirectionalLight::displayImguiWidgets() noexcept
{
#ifndef FINAL_RELEASE
	if ( ImGui::Begin( m_name.c_str() ) )
	{
		bool bDirtyRot = false;
		const auto dirtyCheckRot = [&bDirtyRot]( const bool bChanged )
		{
			bDirtyRot = bDirtyRot || bChanged;
		};

		auto lightDirViewSpaceDeg = util::toDegrees3( m_pscbData.cb_lightPosViewSpace );

		ImGui::Text( "Direction" );
		dirtyCheckRot( ImGui::SliderFloat( "x-Pitch", &lightDirViewSpaceDeg.x, -89.8f, 89.8f, "%.1f" ) );
		dirtyCheckRot( ImGui::SliderFloat( "y-Yaw", &lightDirViewSpaceDeg.y, -89.8f, 89.8f, "%.1f" ) );
		dirtyCheckRot( ImGui::SliderFloat( "z-Roll", &lightDirViewSpaceDeg.z, -89.8f, 89.8f, "%.1f" ) );

		if ( bDirtyRot )
		{
			m_pscbData.cb_lightPosViewSpace = util::toRadians3( lightDirViewSpaceDeg );
			setRotation( m_pscbData.cb_lightPosViewSpace );
		}

		ImGui::Text( "Intensity & Color" );
		ImGui::SliderFloat( "Intensity", &m_pscbData.cb_intensity, 0.01f, 4.0f, "%.2f", 2.0f );
		ImGui::ColorEdit3( "Diffuse", &m_pscbData.cb_lightColor.x );	// #TODO: expose model's material properties cb to change its color when changing the light's color here

		ImGui::Text( "Attenuation" );
		ImGui::SliderFloat( "Constant", &m_pscbData.cb_attConstant, 0.05f, 10.0f, "%.2f", 4.0f );
		ImGui::SliderFloat( "Linear", &m_pscbData.cb_attLinear, 0.0001f, 4.0f, "%.4f", 8.0f );
		ImGui::SliderFloat( "Quadratic", &m_pscbData.cb_attQuadratic, 0.0000001f, 10.0f, "%.7f", 10.0f);

		ImGui::Checkbox( "Show Sphere Mesh", &m_bShowMesh );
	}
	ImGui::End();
#endif
}

void DirectionalLight::setIntensity( const float newIntensity ) noexcept
{
	m_pscbData.cb_intensity = newIntensity;
}

void DirectionalLight::setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept
{
	m_pscbData.cb_lightColor = diffuseColor;
}

DirectX::XMFLOAT3 DirectionalLight::getPosition() const noexcept
{
	return m_lightMesh.getPosition();
}

void DirectionalLight::populateVscbData( Graphics &gfx ) cond_noex
{
	ASSERT( m_bShadowCasting && m_pCameraForShadowing, "Camera not specified (null)!" );
	m_vscbData = {dx::XMMatrixTranspose( m_pCameraForShadowing->getViewMatrix() * m_pCameraForShadowing->getProjectionMatrix( gfx, true, m_shadowCamFarZ ) )};
}

void DirectionalLight::populatePscbData( Graphics &gfx ) cond_noex
{
	static CameraManager &s_cameraMan = CameraManager::getInstance();
	const auto &activeCameraViewMat = s_cameraMan.getActiveCamera().getViewMatrix();
	m_pscbDataToBind = m_pscbData;
	const auto lightDirWorldSpace = DirectX::XMLoadFloat3( &m_pscbData.cb_lightPosViewSpace );
	DirectX::XMStoreFloat3( &m_pscbDataToBind.cb_lightPosViewSpace, DirectX::XMVector3Transform( lightDirWorldSpace, activeCameraViewMat ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SpotLight::SpotLight( Graphics &gfx,
	const float radiusScale /*= 1.0f*/,
	const DirectX::XMFLOAT3 &directionVector /*= {0.0f, 0.0f, 1.0f}*/,
	const DirectX::XMFLOAT3 &pos /*= {8.0f, 8.0f, 2.f}*/,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/,
	const float intensity /*= 1.0f*/,
	const float fovConeAngle /*= 60.0f*/,
	const float shadowCamNearZ /*= 0.5f*/,
	const float shadowCamFarZ /*= 80.0f*/ )
	:
	ILightSource(gfx, LightSourceType::Spot, Model(std::make_unique<Sphere>(gfx, radiusScale, colorOrTexturePath), gfx, Camera::computePitchYawRollInDegFromDirectionVector(directionVector), pos), colorOrTexturePath, bShadowCasting, bShowMesh, directionVector, pos, intensity, fovConeAngle, shadowCamFarZ)
{
	if ( bShadowCasting )
	{
		const std::pair<float, float> pitchAndYaw = Camera::computePitchYawInDegFromDirectionVector( directionVector );

		// set 1.0 aspect ratio for a Spot Light to have a square frustum; the spotlight is typically symmetric (meaning it has the same horizontal and vertical FOV)
		m_pCameraForShadowing = std::make_unique<Camera>( gfx, shadowCamFarZ, shadowCamFarZ, fovConeAngle, pos, pitchAndYaw.first, pitchAndYaw.second, true, true, shadowCamNearZ, shadowCamFarZ );
	}
}

void SpotLight::displayImguiWidgets() noexcept
{
	pass_;
}

void SpotLight::setIntensity( const float newIntensity ) noexcept
{
	m_pscbData.cb_intensity = newIntensity;
}

void SpotLight::setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept
{
	m_pscbData.cb_lightColor = diffuseColor;
}

DirectX::XMFLOAT3 SpotLight::getPosition() const noexcept
{
	return m_pscbData.cb_lightPosViewSpace;
}

void SpotLight::populateVscbData( Graphics &gfx ) cond_noex
{
	ASSERT( m_bShadowCasting && m_pCameraForShadowing, "Camera not specified (null)!" );
	m_vscbData = {dx::XMMatrixTranspose( m_pCameraForShadowing->getViewMatrix() * m_pCameraForShadowing->getProjectionMatrix( gfx, true, m_shadowCamFarZ ) )};
}

void SpotLight::populatePscbData( Graphics &gfx ) cond_noex
{
	pass_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
PointLight::PointLight( Graphics &gfx,
	const float radiusScale /*= 0.5f*/,
	const DirectX::XMFLOAT3 &lightMeshRotationDeg /*= {0.0f, 0.0f, 0.f}*/,
	const DirectX::XMFLOAT3 &pos /*= {8.0f, 8.0f, 2.f}*/,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/,
	const float intensity /*= 1.0f*/,
	const float shadowCamNearZ /*= 1.0f*/,
	const float shadowCamFarZ /*= 100.0f*/ )
	:
	ILightSource(gfx, LightSourceType::Point, Model(std::make_unique<Sphere>(gfx, radiusScale, colorOrTexturePath), gfx, lightMeshRotationDeg, pos), colorOrTexturePath, bShadowCasting, bShowMesh, lightMeshRotationDeg, pos, intensity, 0.0f, shadowCamFarZ)
{
	if ( bShadowCasting )
	{
		m_pCameraForShadowing = std::make_unique<Camera>( gfx, gfx.getClientWidth(), gfx.getClientHeight(), g_pointLightFovDeg, pos, lightMeshRotationDeg.x, lightMeshRotationDeg.y, true, true, shadowCamNearZ, shadowCamFarZ );
	}
}

void PointLight::displayImguiWidgets() noexcept
{
#ifndef FINAL_RELEASE
	if ( ImGui::Begin( m_name.c_str() ) )
	{
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
		ImGui::SliderFloat( "Intensity", &m_pscbData.cb_intensity, 0.01f, 4.0f, "%.2f", 2.0f );
		ImGui::ColorEdit3( "Diffuse", &m_pscbData.cb_lightColor.x );	// #TODO: expose model's material properties cb to change its color when changing the light's color here

		ImGui::Text( "Attenuation" );
		ImGui::SliderFloat( "Constant", &m_pscbData.cb_attConstant, 0.05f, 10.0f, "%.2f", 4.0f );
		ImGui::SliderFloat( "Linear", &m_pscbData.cb_attLinear, 0.0001f, 4.0f, "%.4f", 8.0f );
		ImGui::SliderFloat( "Quadratic", &m_pscbData.cb_attQuadratic, 0.0000001f, 10.0f, "%.7f", 10.0f);

		ImGui::Checkbox( "Show Sphere Mesh", &m_bShowMesh );
	}
	ImGui::End();
#endif
}

void PointLight::setIntensity( const float newIntensity ) noexcept
{
	m_pscbData.cb_intensity = newIntensity;
}

void PointLight::setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept
{
	m_pscbData.cb_lightColor = diffuseColor;
}

DirectX::XMFLOAT3 PointLight::getPosition() const noexcept
{
	return m_pscbData.cb_lightPosViewSpace;
}

void PointLight::populateVscbData( Graphics &gfx ) cond_noex
{
	ASSERT( m_bShadowCasting && m_pCameraForShadowing, "Camera not specified (null)!" );
	const auto &pos = m_pCameraForShadowing->getPosition();
	m_vscbData = {dx::XMMatrixTranspose( dx::XMMatrixTranslation( -pos.x, -pos.y, -pos.z ) )};
}

void PointLight::populatePscbData( Graphics &gfx ) cond_noex
{
	static CameraManager &s_cameraMan = CameraManager::getInstance();
	const auto &activeCameraViewMat = s_cameraMan.getActiveCamera().getViewMatrix();
	m_pscbDataToBind = m_pscbData;
	const auto lightPosWorldSpace = DirectX::XMLoadFloat3( &m_pscbData.cb_lightPosViewSpace );
	DirectX::XMStoreFloat3( &m_pscbDataToBind.cb_lightPosViewSpace, DirectX::XMVector3Transform( lightPosWorldSpace, activeCameraViewMat ) );
}