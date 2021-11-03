#include "light_vcb.h"
#include "camera.h"
#include "assertions_console.h"


namespace dx = DirectX;

LightVCB::LightVCB( Graphics& gph,
	unsigned slot )
	:
	m_pVcb{std::make_unique<VertexConstantBuffer<LightTransformVCB>>( gph, slot )}
{

}

void LightVCB::update( Graphics& gph )
{
	ASSERT( m_pPointLightShadowCamera, "Camera not specified (null)!" );
	const auto pos = m_pPointLightShadowCamera->getPosition();
	const LightTransformVCB vcb{dx::XMMatrixTranspose( dx::XMMatrixTranslation( -pos.x,
		-pos.y,
		-pos.z ) )};
	m_pVcb->update( gph,
		vcb );
}

void LightVCB::bind( Graphics& gph ) cond_noex
{
	m_pVcb->bind( gph );
}

void LightVCB::setCamera( const Camera* pCam ) noexcept
{
	m_pPointLightShadowCamera = pCam;
}