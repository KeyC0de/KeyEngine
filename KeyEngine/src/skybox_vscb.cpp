#include "skybox_vscb.h"


SkyboxVSCB::SkyboxVSCB( Graphics &gph,
	unsigned slot )
	:
	m_pVscb{std::make_unique<VertexShaderConstantBuffer<Transform>>( gph, slot )}
{

}

void SkyboxVSCB::bind( Graphics &gph ) cond_noex
{
	m_pVscb->update( gph,
		getTransform( gph ) );
	m_pVscb->bind( gph );
}

SkyboxVSCB::Transform SkyboxVSCB::getTransform( Graphics &gph ) cond_noex
{
	return {DirectX::XMMatrixTranspose( gph.getViewMatrix() * gph.getProjectionMatrix() )};
}