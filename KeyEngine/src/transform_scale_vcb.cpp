#include "transform_scale_vcb.h"
#include "effect_visitor.h"


namespace dx = DirectX;

TransformScaleVCB::TransformScaleVCB( Graphics& gph,
	unsigned slot,
	float scale )
	:
	TransformVCB{gph, slot},
	m_cbScale{createCbLayout()}
{
	m_cbScale["scale"] = scale;
}

void TransformScaleVCB::accept( IEffectVisitor& ev )
{
	ev.visit( m_cbScale );
}

void TransformScaleVCB::bind( Graphics& gph ) cond_noex
{
	const float scale = m_cbScale["scale"];
	const auto scaleMatrix = dx::XMMatrixScaling( scale,
		scale,
		scale );
	auto tfs = getTransforms( gph );
	tfs.worldView = tfs.worldView * scaleMatrix;
	tfs.worldViewProjection = tfs.worldViewProjection * scaleMatrix;
	updateVcb( gph,
		tfs );
	m_pVcb->bind( gph );
}

std::unique_ptr<IBindableCloning> TransformScaleVCB::clone() const noexcept
{
	return std::make_unique<TransformScaleVCB>( *this );
}

con::RawLayout TransformScaleVCB::createCbLayout()
{
	con::RawLayout layout;
	layout.add<con::Float>( "scale" );
	return layout;
}