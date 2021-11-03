#pragma once

#include "transform_vcb.h"
#include "dynamic_constant_buffer.h"


class TransformScaleVCB
	: public TransformVCB
{
	con::Buffer m_cbScale;
public:
	TransformScaleVCB( Graphics& gph, unsigned slot, float scale );

	void accept( IEffectVisitor& ev ) override;
	void bind( Graphics& gph ) cond_noex override;
	std::unique_ptr<IBindableCloning> clone() const noexcept override;
private:
	static con::RawLayout createCbLayout();
};