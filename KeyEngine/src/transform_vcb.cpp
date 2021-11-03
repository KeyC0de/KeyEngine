#include "transform_vcb.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"


TransformVCB::TransformVCB( Graphics& gph,
	unsigned slot )
{
	if ( !m_pVcb )
	{
		m_pVcb = std::make_unique<VertexConstantBuffer<Transforms>>( gph,
			slot );
	}
}

void TransformVCB::bind( Graphics& gph ) cond_noex
{
	updateVcb( gph,
		getTransforms( gph ) );
	m_pVcb->bind( gph );
}

void TransformVCB::setParentDrawable( const Drawable& parent ) noexcept
{
	m_pDrawable = &parent;
}

std::unique_ptr<IBindableCloning> TransformVCB::clone() const noexcept
{
	return std::make_unique<TransformVCB>( *this );
}

void TransformVCB::updateVcb( Graphics& gph,
	const Transforms& tfs ) cond_noex
{
	ASSERT( m_pDrawable != nullptr, "No Drawable set!" );
	m_pVcb->update( gph,
		tfs );
}

TransformVCB::Transforms TransformVCB::getTransforms( Graphics& gph ) cond_noex
{
	ASSERT( m_pDrawable != nullptr, "No Drawable set!" );
	const auto world = m_pDrawable->getTransform();
	const auto worldView = world * gph.getViewMatrix();
	const auto worldViewProjection = worldView * gph.getProjectionMatrix();
	return {DirectX::XMMatrixTranspose( world ),
			DirectX::XMMatrixTranspose( worldView ),
			DirectX::XMMatrixTranspose( worldViewProjection )};
}