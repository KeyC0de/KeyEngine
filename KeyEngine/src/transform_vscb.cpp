#include "transform_vscb.h"
#include "constant_buffer.h"
#include "graphics.h"
#include "node.h"
#include "mesh.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"


namespace dx = DirectX;

TransformVSCB::TransformVSCB( Graphics &gfx,
	const unsigned slot,
	const Mesh &mesh )
	:
	m_pMesh{&mesh},
	m_pVscb{std::make_unique<VertexShaderConstantBuffer<Transforms>>( gfx, slot )}
{

}

TransformVSCB::TransformVSCB( Graphics &gfx,
	const unsigned slot )
	:
	m_pVscb{std::make_unique<VertexShaderConstantBuffer<Transforms>>( gfx, slot )}
{

}

TransformVSCB::TransformVSCB( const TransformVSCB &rhs )
	:
	m_pMesh{rhs.m_pMesh}
{
	m_pVscb = std::make_unique<VertexShaderConstantBuffer<Transforms>>( VertexShaderConstantBuffer<Transforms>::makeCopy( rhs.m_pVscb->getCb(), rhs.m_pVscb->getSlot() ) );
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

void TransformVSCB::bind( Graphics &gfx ) cond_noex
{
	update( gfx, getTransforms( gfx ) );
	bindCb( gfx );
}

std::unique_ptr<IBindableCloning> TransformVSCB::clone() const noexcept
{
	return std::make_unique<TransformVSCB>( *this );
}

std::unique_ptr<IBindableCloning> TransformVSCB::clone() noexcept
{
	return std::make_unique<TransformVSCB>( std::move( *this ) );
}

void TransformVSCB::update( Graphics &gfx,
	const Transforms &transforms ) cond_noex
{
	m_pVscb->update( gfx, transforms );
}

void TransformVSCB::bindCb( Graphics &gfx ) cond_noex
{
	ASSERT( m_pMesh != nullptr, "No Mesh set!" );
	m_pVscb->bind( gfx );
}

TransformVSCB::Transforms TransformVSCB::getTransforms( Graphics &gfx ) cond_noex
{
	ASSERT( m_pMesh != nullptr, "No Mesh set!" );
	const auto world = m_pMesh->getNode()->getWorldTransform();
	const auto worldView = world * gfx.getViewMatrix();
	const auto worldViewProjection = worldView * gfx.getProjectionMatrix();
	return {dx::XMMatrixTranspose( world ), dx::XMMatrixTranspose( worldView ), dx::XMMatrixTranspose( worldViewProjection )};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
TransformScaleVSCB::TransformScaleVSCB( Graphics &gfx,
	const unsigned slot,
	const float scale )
	:
	TransformVSCB{gfx, slot},
	m_cbScale{calcCbLayout()}
{
	m_cbScale["scale"] = scale;
}

TransformScaleVSCB::TransformScaleVSCB( const TransformScaleVSCB &rhs )
	:
	TransformVSCB{rhs},
	m_cbScale{rhs.m_cbScale}
{

}

TransformScaleVSCB& TransformScaleVSCB::operator=( const TransformScaleVSCB &rhs )
{
	TransformVSCB::operator=( rhs );
	m_cbScale.copyFrom( rhs.m_cbScale );
	return *this;
}

TransformScaleVSCB::TransformScaleVSCB( TransformScaleVSCB &&rhs ) noexcept
	:
	TransformVSCB{std::move( rhs )},
	m_cbScale{std::move( rhs.m_cbScale )}
{

}

TransformScaleVSCB& TransformScaleVSCB::operator=( TransformScaleVSCB &&rhs ) noexcept
{
	TransformVSCB::operator=( std::move( rhs ) );
	m_cbScale.moveFrom( rhs.m_cbScale );
	return *this;
}

#ifndef FINAL_RELEASE
void TransformScaleVSCB::accept( IImGuiConstantBufferVisitor &ev )
{
	ev.visit( m_cbScale );
}
#endif

void TransformScaleVSCB::bind( Graphics &gfx ) cond_noex
{
	const float scale = m_cbScale["scale"];
	const auto scaleMatrix = dx::XMMatrixScaling( scale, scale, scale );
	auto transforms = getTransforms( gfx );
	transforms.worldView = transforms.worldView * scaleMatrix;
	transforms.worldViewProjection = transforms.worldViewProjection * scaleMatrix;
	update(gfx, transforms );
	bindCb( gfx );
}

void TransformScaleVSCB::setMesh( const Mesh &mesh ) noexcept
{
	m_pMesh = &mesh;
}

std::unique_ptr<IBindableCloning> TransformScaleVSCB::clone() const noexcept
{
	return std::make_unique<TransformScaleVSCB>( *this );
}

std::unique_ptr<IBindableCloning> TransformScaleVSCB::clone() noexcept
{
	return std::make_unique<TransformScaleVSCB>( std::move( *this ) );
}

con::RawLayout TransformScaleVSCB::calcCbLayout()
{
	con::RawLayout layout;
	layout.add<con::Float>( "scale" );
	return layout;
}
