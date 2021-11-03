#include "skybox_vcb.h"


SkyboxVCB::SkyboxVCB( Graphics& gph,
	unsigned slot )
	:
	m_pVcb{std::make_unique<VertexConstantBuffer<Transform>>( gph, slot )}
{

}

void SkyboxVCB::bind( Graphics& gph ) cond_noex
{
	m_pVcb->update( gph,
		getTransform( gph ) );
	m_pVcb->bind( gph );
}

SkyboxVCB::Transform SkyboxVCB::getTransform( Graphics& gph ) cond_noex
{
	return {DirectX::XMMatrixTranspose( gph.getViewMatrix() * gph.getProjectionMatrix() )};
}