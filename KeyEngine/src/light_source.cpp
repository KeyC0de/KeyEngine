#include "light_source.h"
#include "sphere.h"
#include "graphics.h"
#include "camera.h"
#include "math_utils.h"
#include "d3d_utils.h"
#include "assertions_console.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
#endif


namespace dx = DirectX;

/*
void DirectionalLightVSCB::update( Graphics &gfx )
{
	ASSERT( m_pDirectionalLightShadowCamera, "Camera not specified (null)!" );
	dx::XMFLOAT3 dir;
	dx::XMStoreFloat3( &dir, m_pDirectionalLightShadowCamera->getDirection() );
	const DirectionalPointLightSourceShadowTransformVSCB vscb{dx::XMMatrixTranspose( dx::XMMatrixTranslation( -dir.x, -dir.y, -dir.z ) )};
	m_pVscb->update( gfx, vscb );
}
*/


PointLight::PointLight( Graphics &gfx,
	const float radiusScale /*= 0.5f*/,
	const DirectX::XMFLOAT3 &initialRotDeg /*= {0.0f, 0.0f, 0.f}*/,
	const DirectX::XMFLOAT3 &pos /*= {8.0f, 8.0f, 2.f}*/,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/,
	const bool bShadowCasting /*= true*/,
	const bool bShowMesh /*= true*/ )
	:
	m_rot{util::toRadians3( initialRotDeg )},
	m_sphereMesh(std::make_unique<Sphere>(gfx, radiusScale, colorOrTexturePath), gfx, initialRotDeg, pos),
	m_pscb(gfx, s_pointLightPscbSlot),
	m_bCastingShadows{bShadowCasting},
	m_bShowMesh{bShowMesh}
{
	static int s_id = 0;
	++s_id;
	m_name = std::string{"Light#"} + std::to_string( s_id );

	const dx::XMFLOAT4 lightColor = std::holds_alternative<dx::XMFLOAT4>( colorOrTexturePath ) ? std::get<dx::XMFLOAT4>( colorOrTexturePath ) : dx::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f};
	m_pscbDataDefault = {pos,
		{0.08f, 0.08f, 0.08f},
		{lightColor.x, lightColor.y, lightColor.z},
		1.0f,
		1.0f,
		0.025f,
		0.0030f};
	resetToDefault();
	if ( bShadowCasting )
	{
		m_pCameraForShadowing = std::make_shared<Camera>( gfx, 90.0f, m_pscbData.pos, initialRotDeg.x, initialRotDeg.y, true );
	}
}

// #TODO: rework this
void PointLight::update( Graphics &gfx,
	const float dt,
	const DirectX::XMMATRIX &activeCameraViewMat,
	const bool bEnableSmoothMovement /*= false*/ ) cond_noex
{
	auto copy = m_pscbData;
	const auto lightPos = DirectX::XMLoadFloat3( &m_pscbData.pos );
	DirectX::XMStoreFloat3( &copy.pos, DirectX::XMVector3Transform( lightPos, activeCameraViewMat ) );
	m_pscb.update( gfx, copy );
	if ( m_bShowMesh )
	{
		m_sphereMesh.setTranslation( m_pscbData.pos );
		m_sphereMesh.setRotation( m_rot );
	}
	m_pscb.bind( gfx );
}

void PointLight::render( const size_t channels ) const cond_noex
{
	if ( m_bShowMesh )
	{
		m_sphereMesh.render( channels );
	}
}

void PointLight::connectMaterialsToRenderer( ren::Renderer &r )
{
	m_sphereMesh.connectMaterialsToRenderer( r );
}

void PointLight::setIntensity( const float newIntensity ) noexcept
{
	m_pscbData.intensity = newIntensity;
}

void PointLight::setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept
{
	m_pscbData.lightColor = diffuseColor;
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
			m_pCameraForShadowing->setRotation( m_rot );
			if ( m_bShowMesh )
			{
				m_sphereMesh.setRotation( m_rot );
			}
		}

		bool bDirtyPos = false;
		const auto dirtyCheckPos = [&bDirtyPos]( const bool bChanged )
		{
			bDirtyPos = bDirtyPos || bChanged;
		};
		
		ImGui::Text( "Position" );
		dirtyCheckPos( ImGui::SliderFloat( "X", &m_pscbData.pos.x, -100.0f, 100.0f, "%.1f" ) );
		dirtyCheckPos( ImGui::SliderFloat( "Y", &m_pscbData.pos.y, -100.0f, 100.0f, "%.1f" ) );
		dirtyCheckPos( ImGui::SliderFloat( "Z", &m_pscbData.pos.z, -100.0f, 100.0f, "%.1f" ) );

		if ( bDirtyPos )
		{
			m_pCameraForShadowing->setTranslation( m_pscbData.pos );
			if ( m_bShowMesh )
			{
				m_sphereMesh.setTranslation( m_pscbData.pos );
			}
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

		if ( ImGui::Button( "Reset" ) )
		{
			resetToDefault();
		}
	}
	ImGui::End();
#endif
}

void PointLight::resetToDefault() noexcept
{
	m_pscbData = m_pscbDataDefault;
}

std::shared_ptr<Camera> PointLight::shareCamera() const noexcept
{
	ASSERT( m_pCameraForShadowing, "There is no shadow camera!" );
	return m_pCameraForShadowing;
}

bool PointLight::isCastingShadows() const noexcept
{
	return m_bCastingShadows;
}

std::string PointLight::getName() const noexcept
{
	return m_name;
}
