#include "light_source.h"
#include "sphere.h"
#include "graphics.h"
#include "camera.h"
#include "camera_manager.h"
#include "math_utils.h"
#include "d3d_utils.h"
#include "assertions_console.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
#endif


namespace dx = DirectX;

ILightSource::ILightSource( Model model,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/,
	const DirectX::XMFLOAT3 &initialRotDeg /*= {0.0f, 0.0f, 0.f}*/ )
	:
	m_bShowMesh{bShowMesh},
	m_bCastingShadows{bShadowCasting},
	m_rot{util::toRadians3( initialRotDeg )},
	m_lightMesh(std::move( model) )
{
	static int s_id = 0;
	++s_id;
	m_name = std::string{"Light#"} + std::to_string( s_id );
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

void ILightSource::connectMaterialsToRenderer( ren::Renderer &r )
{
	m_lightMesh.connectMaterialsToRenderer( r );
}

std::shared_ptr<Camera> ILightSource::shareCamera() const noexcept
{
	ASSERT( m_pCameraForShadowing, "There is no shadow camera!" );
	return m_pCameraForShadowing;
}

bool ILightSource::isCastingShadows() const noexcept
{
	return m_bCastingShadows;
}

std::string ILightSource::getName() const noexcept
{
	return m_name;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
DirectionalLight::DirectionalLight( Graphics &gfx,
	const float radiusScale /*= 1.0f*/,
	const DirectX::XMFLOAT3 &initialRotDeg /*= {0.0f, 0.0f, 0.f}*/,
	const DirectX::XMFLOAT3 &pos /*= {8.0f, 8.0f, 2.f}*/,
	const std::variant<DirectX::XMFLOAT4,std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/ )
	:
	ILightSource(Model(std::make_unique<Sphere>(gfx, radiusScale, colorOrTexturePath), gfx, initialRotDeg, pos), bShadowCasting, bShowMesh, initialRotDeg),
	m_pscb(gfx, s_directionalLightPscbSlot)
{

}

void DirectionalLight::update( Graphics &gfx,
	const float dt,
	const float lerpBetweenFrames,
	const bool bEnableSmoothMovement /*= false*/ ) cond_noex
{
	/*
	ASSERT( m_pDirectionalLightShadowCamera, "Camera not specified (null)!" );
	dx::XMFLOAT3 dir;
	dx::XMStoreFloat3( &dir, m_pDirectionalLightShadowCamera->getDirection() );
	const DirectionalPointLightSourceShadowTransformVSCB vscb{dx::XMMatrixTranspose( dx::XMMatrixTranslation( -dir.x, -dir.y, -dir.z ) )};
	m_pVscb->update( gfx, vscb );
	*/
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

////////////////////////////////////////////////////////////////////////////////////////////////////
PointLight::PointLight( Graphics &gfx,
	const float radiusScale /*= 0.5f*/,
	const DirectX::XMFLOAT3 &initialRotDeg /*= {0.0f, 0.0f, 0.f}*/,
	const DirectX::XMFLOAT3 &pos /*= {8.0f, 8.0f, 2.f}*/,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/ )
	:
	ILightSource(Model(std::make_unique<Sphere>(gfx, radiusScale, colorOrTexturePath), gfx, initialRotDeg, pos), bShadowCasting, bShowMesh, initialRotDeg),
	m_pscb(gfx, s_pointLightPscbSlot)
{
	const dx::XMFLOAT4 lightColor = std::holds_alternative<dx::XMFLOAT4>( colorOrTexturePath ) ? std::get<dx::XMFLOAT4>( colorOrTexturePath ) : dx::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f};
	m_pscbData = {pos,
		{0.08f, 0.08f, 0.08f},
		{lightColor.x, lightColor.y, lightColor.z},
		1.0f,
		1.0f,
		0.025f,
		0.0030f};
	if ( bShadowCasting )
	{
		m_pCameraForShadowing = std::make_shared<Camera>( gfx, 90.0f, m_pscbData.pointLightPosViewSpace, initialRotDeg.x, initialRotDeg.y, true );
	}
}

void PointLight::update( Graphics &gfx,
	const float dt,
	const float lerpBetweenFrames,
	const bool bEnableSmoothMovement /*= false*/ ) cond_noex
{
	static CameraManager &s_cameraMan = CameraManager::getInstance();
	const auto &activeCameraViewMat = s_cameraMan.getActiveCamera().getViewMatrix();

	auto copy = m_pscbData;
	const auto lightPosViewSpace = DirectX::XMLoadFloat3( &m_pscbData.pointLightPosViewSpace );

	DirectX::XMStoreFloat3( &copy.pointLightPosViewSpace, DirectX::XMVector3Transform( lightPosViewSpace, activeCameraViewMat ) );
	m_pscb.update( gfx, copy );
	m_pscb.bind( gfx );

	ILightSource::update( gfx, dt, lerpBetweenFrames, bEnableSmoothMovement );
}

void PointLight::displayImguiWidgets() noexcept
{
#ifndef FINAL_RELEASE
	if ( ImGui::Begin( m_name.c_str() ) )
	{
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
			if ( m_bCastingShadows )
			{
				m_pCameraForShadowing->setRotation( m_rot );
			}
			m_lightMesh.setRotation( m_rot );
		}

		bool bDirtyPos = false;
		const auto dirtyCheckPos = [&bDirtyPos]( const bool bChanged )
		{
			bDirtyPos = bDirtyPos || bChanged;
		};
		
		ImGui::Text( "Position" );
		dirtyCheckPos( ImGui::SliderFloat( "X", &m_pscbData.pointLightPosViewSpace.x, -100.0f, 100.0f, "%.1f" ) );
		dirtyCheckPos( ImGui::SliderFloat( "Y", &m_pscbData.pointLightPosViewSpace.y, -100.0f, 100.0f, "%.1f" ) );
		dirtyCheckPos( ImGui::SliderFloat( "Z", &m_pscbData.pointLightPosViewSpace.z, -100.0f, 100.0f, "%.1f" ) );

		if ( bDirtyPos )
		{
			if ( m_bCastingShadows )
			{
				m_pCameraForShadowing->setTranslation( m_pscbData.pointLightPosViewSpace );
			}
			m_lightMesh.setTranslation( m_pscbData.pointLightPosViewSpace );
		}

		ImGui::Text( "Intensity & Color" );
		ImGui::SliderFloat( "Intensity", &m_pscbData.intensity, 0.01f, 4.0f, "%.2f", 2.0f );
		ImGui::ColorEdit3( "Diffuse", &m_pscbData.lightColor.x );
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

////////////////////////////////////////////////////////////////////////////////////////////////////
SpotLight::SpotLight( Graphics &gfx,
	const float radiusScale /*= 1.0f*/,
	const DirectX::XMFLOAT3 &initialRotDeg /*= {0.0f, 0.0f, 0.f}*/,
	const DirectX::XMFLOAT3 &pos /*= {8.0f, 8.0f, 2.f}*/,
	const std::variant<DirectX::XMFLOAT4,std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const float coneAngle /*= 60.0f*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/ )
	:
	ILightSource(Model(std::make_unique<Sphere>(gfx, radiusScale, colorOrTexturePath), gfx, initialRotDeg, pos), bShadowCasting, bShowMesh, initialRotDeg),
	m_pscb(gfx, s_spotLightPscbSlot)
{

}

void SpotLight::update( Graphics &gfx,
	const float dt,
	const float lerpBetweenFrames,
	const bool bEnableSmoothMovement /*= false*/ ) cond_noex
{
	pass_;
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
