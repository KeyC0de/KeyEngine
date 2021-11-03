#include "light.h"
#include "imgui.h"
#include "camera.h"
#include "math_utils.h"
#include "assertions_console.h"
#include "camera_manager.h"


/*
void DirectionalLightVCB::update( Graphics& gph )
{
	ASSERT( m_pDirectionalLightShadowCamera, "Camera not specified (null)!" );
	dx::XMFLOAT3 dir;
	dx::XMStoreFloat3( &dir,
		m_pDirectionalLightShadowCamera->getDirection() );
	const DirectionalLightTransformVCB vcb{dx::XMMatrixTranspose( dx::XMMatrixTranslation( -dir.x,
		-dir.y,
		-dir.z ) )};
	m_pVcb->update( gph,
		vcb );
}
*/

PointLight::PointLight( Graphics& gph,
	const DirectX::XMFLOAT3& pos,
	const DirectX::XMFLOAT3& col,
	bool bShadowCasting,
	float radius )
	:
	m_sphereMesh(gph, radius),
	m_pcb(gph, m_pointLightPcbSlot),
	m_bShadowCasting{bShadowCasting}
{
	static int id = 0;
	++id;
	m_name = {std::string{"Light"} + std::to_string( id )};
	m_pcbHomeData = {pos,
		{0.08f, 0.08f, 0.08f},
		col,
		1.0f,
		1.0f,
		0.025f,
		0.0030f};
	resetToDefault();
	if ( bShadowCasting )
	{
		m_pShadowCamera = std::make_shared<Camera>( gph,
			std::string{"ShadowCam"} + std::to_string( id ),
			CameraManager::getInstance().getClientWidth(),
			CameraManager::getInstance().getClientHeight(),
			90.0f,
			m_pcbData.pos,
			0.0f,
			math::PI / 2.0f,
			true );
	}
}

std::string PointLight::getName() const noexcept
{
	return m_name;
}

void PointLight::setIntensity( float newIntensity ) noexcept
{
	m_pcbData.intensity = newIntensity;
}

void PointLight::setColor( const DirectX::XMFLOAT3& diffuseColor ) noexcept
{
	m_pcbData.lightColor = diffuseColor;
}

bool PointLight::isCastingShadows() const noexcept
{
	return m_bShadowCasting;
}

void PointLight::displayImguiWidgets() noexcept
{
	if ( ImGui::Begin( m_name.c_str() ) )
	{
		bool bDirty = false;
		const auto dirtyCheck = [&bDirty]( bool bd )
		{
			bDirty = bDirty || bd;
		};

		ImGui::Text( "Position" );
		dirtyCheck( ImGui::SliderFloat( "X", &m_pcbData.pos.x, -60.0f, 60.0f,
			"%.1f" ) );
		dirtyCheck( ImGui::SliderFloat( "Y", &m_pcbData.pos.y, -60.0f, 60.0f,
			"%.1f" ) );
		dirtyCheck( ImGui::SliderFloat( "Z", &m_pcbData.pos.z, -60.0f, 60.0f,
			"%.1f" ) );

		if ( bDirty && m_bShadowCasting )
		{
			m_pShadowCamera->setPosition( m_pcbData.pos );
		}
		
		ImGui::Text( "Intensity & Color" );
		ImGui::SliderFloat( "Intensity", &m_pcbData.intensity, 0.01f, 4.0f,
			"%.2f",
			2.0f );
		ImGui::ColorEdit3( "Diffuse", &m_pcbData.lightColor.x );
		ImGui::ColorEdit3( "Ambient", &m_pcbData.ambient.x );
		
		ImGui::Text( "Attenuation" );
		ImGui::SliderFloat( "Constant", &m_pcbData.attConstant, 0.05f, 10.0f, "%.2f",
			4.0f );
		ImGui::SliderFloat( "Linear", &m_pcbData.attLinear, 0.0001f, 4.0f, "%.4f",
			8.0f );
		ImGui::SliderFloat( "Quadratic", &m_pcbData.attQuadratic, 0.0000001f, 10.0f,
			"%.7f",
			10.0f);

		if ( ImGui::Button( "Reset" ) )
		{
			resetToDefault();
		}
	}
	ImGui::End();
}

void PointLight::resetToDefault() noexcept
{
	m_pcbData = m_pcbHomeData;
}

void PointLight::update( Graphics& gph,
	float dt,
	const DirectX::XMMATRIX& activeCameraViewMat ) const noexcept
{
	auto copy = m_pcbData;
	const auto lightViewSpacePos = DirectX::XMLoadFloat3( &m_pcbData.pos );
	DirectX::XMStoreFloat3( &copy.pos,
		DirectX::XMVector3Transform( lightViewSpacePos, activeCameraViewMat ) );
	m_pcb.update( gph,
		copy );
	m_sphereMesh.setPosition( m_pcbData.pos );
	m_pcb.bind( gph );
}

void PointLight::render( size_t channels ) const cond_noex
{
	m_sphereMesh.render( channels );
}

void PointLight::connectEffectsToRenderer( ren::Renderer& r )
{
	m_sphereMesh.connectEffectsToRenderer( r );
}

std::shared_ptr<Camera> PointLight::shareCamera() const noexcept
{
	ASSERT( m_pShadowCamera, "There is no shadow camera!" );
	return m_pShadowCamera;
}