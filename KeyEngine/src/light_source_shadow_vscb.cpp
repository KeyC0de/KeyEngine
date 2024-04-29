#include "light_source_shadow_vscb.h"
#include "camera.h"
#include "assertions_console.h"


namespace dx = DirectX;

PointLightSourceShadowVSCB::PointLightSourceShadowVSCB( Graphics &gph,
	const unsigned slot )
	:
	m_pVscb{std::make_unique<VertexShaderConstantBuffer<PointLightSourceShadowTransformVSCB>>( gph, slot )}
{

}

void PointLightSourceShadowVSCB::update( Graphics &gph )
{
	ASSERT( m_pPointLightShadowCamera, "Camera not specified (null)!" );
	const auto &pos = m_pPointLightShadowCamera->getPosition();
	const PointLightSourceShadowTransformVSCB vscb{dx::XMMatrixTranspose( dx::XMMatrixTranslation( -pos.x, -pos.y, -pos.z ) )};
	m_pVscb->update( gph, vscb );
}

void PointLightSourceShadowVSCB::bind( Graphics &gph ) cond_noex
{
	m_pVscb->bind( gph );
}

void PointLightSourceShadowVSCB::setCamera( const Camera *pCam, const bool bEnable ) noexcept
{
	if ( bEnable )
	{
		m_pPointLightShadowCamera = pCam;
	}
	else
	{
		m_pPointLightShadowCamera = nullptr;
	}
}

bool PointLightSourceShadowVSCB::isCastingShadows() const noexcept
{
	return m_pPointLightShadowCamera != nullptr;
}
