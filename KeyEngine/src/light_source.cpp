#include "light_source.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
#endif
#include "camera.h"
#include "math_utils.h"
#include "assertions_console.h"


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
	const DirectX::XMFLOAT3 &pos,
	const DirectX::XMFLOAT3 &col,
	const bool bShadowCasting,
	const bool bShowMesh,
	const float radius )
	:
	m_sphereMesh(gfx, radius),
	m_pscb(gfx, s_pointLightPscbSlot),
	m_bCastingShadows{bShadowCasting},
	m_bShowMesh{bShowMesh}
{
	static int id = 0;
	++id;
	m_name = std::string{"Light#"} + std::to_string( id );
	m_pscbHomeData = {pos, {0.08f, 0.08f, 0.08f}, col, 1.0f, 1.0f, 0.025f, 0.0030f};
	resetToDefault();
	if ( bShadowCasting )
	{
		m_pCameraForShadowing = std::make_shared<Camera>( gfx, std::string{"ShadowCam#"} + std::to_string( id ), gfx.getClientWidth(), gfx.getClientHeight(), 90.0f, m_pscbData.pos, 0.0f, util::PI / 2.0f, true );
	}
}

void PointLight::update( Graphics &gfx,
	const float dt,
	const DirectX::XMMATRIX &activeCameraViewMat ) const noexcept
{
	auto copy = m_pscbData;
	const auto lightPos = DirectX::XMLoadFloat3( &m_pscbData.pos );
	DirectX::XMStoreFloat3( &copy.pos, DirectX::XMVector3Transform( lightPos, activeCameraViewMat ) );
	m_pscb.update( gfx, copy );
	if ( m_bShowMesh )
	{
		m_sphereMesh.setPosition( m_pscbData.pos );
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
		
		ImGui::Text( "Position" );
		dirtyCheckRot( ImGui::SliderFloat( "X", &m_pscbData.pos.x, -60.0f, 60.0f, "%.1f" ) );
		dirtyCheckRot( ImGui::SliderFloat( "Y", &m_pscbData.pos.y, -60.0f, 60.0f, "%.1f" ) );
		dirtyCheckRot( ImGui::SliderFloat( "Z", &m_pscbData.pos.z, -60.0f, 60.0f, "%.1f" ) );

		if ( bDirtyRot && m_bCastingShadows )
		{
			m_pCameraForShadowing->setPosition( m_pscbData.pos );
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
	m_pscbData = m_pscbHomeData;
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

const std::string& PointLight::getName() const noexcept
{
	return m_name;
}