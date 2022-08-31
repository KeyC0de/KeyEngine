#include "light_vscb.h"
#include "camera.h"
#include "assertions_console.h"


namespace dx = DirectX;

LightVSCB::LightVSCB( Graphics &gph,
	const unsigned slot )
	:
	m_pVscb{std::make_unique<VertexShaderConstantBuffer<LightTransformVSCB>>( gph, slot )}
{

}

void LightVSCB::update( Graphics &gph )
{
	ASSERT( m_pPointLightShadowCamera, "Camera not specified (null)!" );
	const auto pos = m_pPointLightShadowCamera->getPosition();
	const LightTransformVSCB vscb{dx::XMMatrixTranspose( dx::XMMatrixTranslation( -pos.x,
		-pos.y,
		-pos.z ) )};
	m_pVscb->update( gph,
		vscb );
}

void LightVSCB::bind( Graphics &gph ) cond_noex
{
	m_pVscb->bind( gph );
}

void LightVSCB::setCamera( const Camera *pCam ) noexcept
{
	m_pPointLightShadowCamera = pCam;
}