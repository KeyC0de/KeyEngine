#include "transform_vscb.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"


TransformVSCB::TransformVSCB( Graphics &gph,
	const unsigned slot )
	:
	m_pVscb{std::make_unique<VertexShaderConstantBuffer<Transforms>>( gph, slot )}
{

}

TransformVSCB::TransformVSCB( const TransformVSCB &rhs )
	:
	m_pMesh{rhs.m_pMesh}
{
	m_pVscb = std::make_unique<VertexShaderConstantBuffer<Transforms>>( VertexShaderConstantBuffer<Transforms>::makeACopy( rhs.m_pVscb->getCb(), rhs.m_pVscb->getSlot() ) );
}

TransformVSCB& TransformVSCB::operator=( const TransformVSCB &rhs )
{
	TransformVSCB temp{rhs};
	std::swap( *this, temp );
	return *this;
}

TransformVSCB::TransformVSCB( TransformVSCB &&rhs ) noexcept
	:
	m_pMesh{rhs.m_pMesh},
	m_pVscb{std::move( rhs.m_pVscb )}
{

}

TransformVSCB& TransformVSCB::operator=( TransformVSCB &&rhs ) noexcept
{
	TransformVSCB temp{std::move( rhs )};
	std::swap( *this, temp );
	return *this;
}

void TransformVSCB::bind( Graphics &gph ) cond_noex
{
	update( gph, getTransforms( gph ) );
	bindCb( gph );
}

void TransformVSCB::setMesh( const Mesh &mesh ) noexcept
{
	m_pMesh = &mesh;
}

std::unique_ptr<IBindableCloning> TransformVSCB::clone() const noexcept
{
	return std::make_unique<TransformVSCB>( *this );
}

std::unique_ptr<IBindableCloning> TransformVSCB::clone() noexcept
{
	return std::make_unique<TransformVSCB>( std::move( *this ) );
}

void TransformVSCB::update( Graphics &gph,
	const Transforms &transforms ) cond_noex
{
	m_pVscb->update( gph, transforms );
}

void TransformVSCB::bindCb( Graphics &gph ) cond_noex
{
	ASSERT( m_pMesh != nullptr, "No Mesh set!" );
	m_pVscb->bind( gph );
}

TransformVSCB::Transforms TransformVSCB::getTransforms( Graphics &gph ) cond_noex
{
	ASSERT( m_pMesh != nullptr, "No Mesh set!" );
	const auto world = m_pMesh->getTransform();
	const auto worldView = world * gph.getViewMatrix();
	const auto worldViewProjection = worldView * gph.getProjectionMatrix();
	// TODO: try removing transpose
	return {DirectX::XMMatrixTranspose( world ), DirectX::XMMatrixTranspose( worldView ), DirectX::XMMatrixTranspose( worldViewProjection )};
}