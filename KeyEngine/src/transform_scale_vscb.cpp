#include "transform_scale_vscb.h"
#include "imgui_visitors.h"


namespace dx = DirectX;

TransformScaleVSCB::TransformScaleVSCB( Graphics &gph,
	const unsigned slot,
	const float scale )
	:
	TransformVSCB{gph, slot},
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

void TransformScaleVSCB::accept( IImGuiEffectVisitor &ev )
{
	ev.visit( m_cbScale );
}

void TransformScaleVSCB::bind( Graphics &gph ) cond_noex
{
	const float scale = m_cbScale["scale"];
	const auto scaleMatrix = dx::XMMatrixScaling( scale, scale, scale );
	auto transforms = getTransforms( gph );
	transforms.worldView = transforms.worldView * scaleMatrix;
	transforms.worldViewProjection = transforms.worldViewProjection * scaleMatrix;
	update(gph, transforms );
	bindCb( gph );
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