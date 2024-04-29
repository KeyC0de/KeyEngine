#include "sky_vscb.h"


SkyVSCB::SkyVSCB( Graphics &gph,
	const unsigned slot )
	:
	m_pVscb{std::make_unique<VertexShaderConstantBuffer<Transform>>( gph, slot )}
{

}

void SkyVSCB::bind( Graphics &gph ) cond_noex
{
	m_pVscb->update( gph, getTransform( gph ) );
	m_pVscb->bind( gph );
}

SkyVSCB::Transform SkyVSCB::getTransform( Graphics &gph ) cond_noex
{
	return {DirectX::XMMatrixTranspose( gph.getViewMatrix() * gph.getProjectionMatrix() )};
}