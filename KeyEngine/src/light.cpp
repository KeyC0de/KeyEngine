#include "light.h"
#include "imgui.h"
#include "camera.h"
#include "math_utils.h"
#include "assertions_console.h"
#include "camera_manager.h"


/*
void DirectionalLightVSCB::update( Graphics &gph )
{
	ASSERT( m_pDirectionalLightShadowCamera, "Camera not specified (null)!" );
	dx::XMFLOAT3 dir;
	dx::XMStoreFloat3( &dir,
		m_pDirectionalLightShadowCamera->getDirection() );
	const DirectionalLightTransformVSCB vscb{dx::XMMatrixTranspose( dx::XMMatrixTranslation( -dir.x,
		-dir.y,
		-dir.z ) )};
	m_pVscb->update( gph,
		vscb );
}
*/

PointLight::PointLight( Graphics &gph,
	const DirectX::XMFLOAT3 &pos,
	const DirectX::XMFLOAT3 &col,
	bool bShadowCasting,
	float radius )
	:
	m_sphereMesh(gph, radius),
	m_pscb(gph, m_pointLightPscbSlot),
	m_bShadowCasting{bShadowCasting}
{
	static int id = 0;
	++id;
	m_name = {std::string{"Light"} + std::to_string( id )};
	m_pscbHomeData = {pos,
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
			m_pscbData.pos,
			0.0f,
			util::PI / 2.0f,
			true );
	}
}

std::string PointLight::getName() const noexcept
{
	return m_name;
}

void PointLight::setIntensity( float newIntensity ) noexcept
{
	m_pscbData.intensity = newIntensity;
}

void PointLight::setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept
{
	m_pscbData.lightColor = diffuseColor;
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
		dirtyCheck( ImGui::SliderFloat( "X", &m_pscbData.pos.x, -60.0f, 60.0f,
			"%.1f" ) );
		dirtyCheck( ImGui::SliderFloat( "Y", &m_pscbData.pos.y, -60.0f, 60.0f,
			"%.1f" ) );
		dirtyCheck( ImGui::SliderFloat( "Z", &m_pscbData.pos.z, -60.0f, 60.0f,
			"%.1f" ) );

		if ( bDirty && m_bShadowCasting )
		{
			m_pShadowCamera->setPosition( m_pscbData.pos );
		}
		
		ImGui::Text( "Intensity & Color" );
		ImGui::SliderFloat( "Intensity", &m_pscbData.intensity, 0.01f, 4.0f,
			"%.2f",
			2.0f );
		ImGui::ColorEdit3( "Diffuse", &m_pscbData.lightColor.x );
		ImGui::ColorEdit3( "Ambient", &m_pscbData.ambient.x );
		
		ImGui::Text( "Attenuation" );
		ImGui::SliderFloat( "Constant", &m_pscbData.attConstant, 0.05f, 10.0f, "%.2f",
			4.0f );
		ImGui::SliderFloat( "Linear", &m_pscbData.attLinear, 0.0001f, 4.0f, "%.4f",
			8.0f );
		ImGui::SliderFloat( "Quadratic", &m_pscbData.attQuadratic, 0.0000001f, 10.0f,
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
	m_pscbData = m_pscbHomeData;
}

void PointLight::update( Graphics &gph,
	float dt,
	const DirectX::XMMATRIX &activeCameraViewMat ) const noexcept
{
	auto copy = m_pscbData;
	const auto lightViewSpacePos = DirectX::XMLoadFloat3( &m_pscbData.pos );
	DirectX::XMStoreFloat3( &copy.pos,
		DirectX::XMVector3Transform( lightViewSpacePos, activeCameraViewMat ) );
	m_pscb.update( gph,
		copy );
	m_sphereMesh.setPosition( m_pscbData.pos );
	m_pscb.bind( gph );
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