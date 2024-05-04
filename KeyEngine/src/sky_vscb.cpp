#include "sky_vscb.h"


SkyVSCB::SkyVSCB( Graphics &gfx,
	const unsigned slot )
	:
	m_pVscb{std::make_unique<VertexShaderConstantBuffer<Transform>>( gfx, slot )}
{

}

void SkyVSCB::bind( Graphics &gfx ) cond_noex
{
	m_pVscb->update( gfx, getTransform( gfx ) );
	m_pVscb->bind( gfx );
}

SkyVSCB::Transform SkyVSCB::getTransform( Graphics &gfx ) cond_noex
{
	return {DirectX::XMMatrixTranspose( gfx.getViewMatrix() * gfx.getProjectionMatrix() )};
}