#include "transform_vscb.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"


TransformVSCB::TransformVSCB( Graphics &gph,
	unsigned slot )
{
	if ( !m_pVscb )
	{
		m_pVscb = std::make_unique<VertexShaderConstantBuffer<Transforms>>( gph,
			slot );
	}
}

void TransformVSCB::bind( Graphics &gph ) cond_noex
{
	update( gph,
		getTransforms( gph ) );
	m_pVscb->bind( gph );
}

void TransformVSCB::setParentDrawable( const Drawable& parent ) noexcept
{
	m_pDrawable = &parent;
}

std::unique_ptr<IBindableCloning> TransformVSCB::clone() const noexcept
{
	return std::make_unique<TransformVSCB>( *this );
}

void TransformVSCB::update( Graphics &gph,
	const Transforms& tfs ) cond_noex
{
	ASSERT( m_pDrawable != nullptr, "No Drawable set!" );
	m_pVscb->update( gph,
		tfs );
}

TransformVSCB::Transforms TransformVSCB::getTransforms( Graphics &gph ) cond_noex
{
	ASSERT( m_pDrawable != nullptr, "No Drawable set!" );
	const auto world = m_pDrawable->getTransform();
	const auto worldView = world * gph.getViewMatrix();
	const auto worldViewProjection = worldView * gph.getProjectionMatrix();
	return {DirectX::XMMatrixTranspose( world ),
			DirectX::XMMatrixTranspose( worldView ),
			DirectX::XMMatrixTranspose( worldViewProjection )};
}