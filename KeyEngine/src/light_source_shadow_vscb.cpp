#include "light_source_shadow_vscb.h"
#include "camera.h"
#include "assertions_console.h"


namespace dx = DirectX;

PointLightSourceShadowVSCB::PointLightSourceShadowVSCB( Graphics &gfx,
	const unsigned slot )
	:
	m_pVscb{std::make_unique<VertexShaderConstantBuffer<PointLightSourceShadowTransformVSCB>>( gfx, slot )}
{

}

void PointLightSourceShadowVSCB::update( Graphics &gfx )
{
	ASSERT( m_pPointLightShadowCamera, "Camera not specified (null)!" );
	const auto &pos = m_pPointLightShadowCamera->getPosition();
	const PointLightSourceShadowTransformVSCB vscb{dx::XMMatrixTranspose( dx::XMMatrixTranslation( -pos.x, -pos.y, -pos.z ) )};
	m_pVscb->update( gfx, vscb );
}

void PointLightSourceShadowVSCB::bind( Graphics &gfx ) cond_noex
{
	m_pVscb->bind( gfx );
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
