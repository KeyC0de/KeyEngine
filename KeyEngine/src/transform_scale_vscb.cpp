#include "transform_scale_vscb.h"
#include "effect_visitor.h"


namespace dx = DirectX;

TransformScaleVSCB::TransformScaleVSCB( Graphics &gph,
	unsigned slot,
	float scale )
	:
	TransformVSCB{gph, slot},
	m_cbScale{createCbLayout()}
{
	m_cbScale["scale"] = scale;
}

void TransformScaleVSCB::accept( IEffectVisitor &ev )
{
	ev.visit( m_cbScale );
}

void TransformScaleVSCB::bind( Graphics &gph ) cond_noex
{
	const float scale = m_cbScale["scale"];
	const auto scaleMatrix = dx::XMMatrixScaling( scale,
		scale,
		scale );
	auto tfs = getTransforms( gph );
	tfs.worldView = tfs.worldView * scaleMatrix;
	tfs.worldViewProjection = tfs.worldViewProjection * scaleMatrix;
	TransformVSCB::update( gph,
		tfs );
	m_pVscb->bind( gph );
}

std::unique_ptr<IBindableCloning> TransformScaleVSCB::clone() const noexcept
{
	return std::make_unique<TransformScaleVSCB>( *this );
}

std::unique_ptr<IBindableCloning> TransformScaleVSCB::clone() noexcept
{
	return std::make_unique<TransformScaleVSCB>( std::move( *this ) );
}

con::RawLayout TransformScaleVSCB::createCbLayout()
{
	con::RawLayout layout;
	layout.add<con::Float>( "scale" );
	return layout;
}