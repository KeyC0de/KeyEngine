#pragma once

#include "transform_vscb.h"
#include "dynamic_constant_buffer.h"


class TransformScaleVSCB
	: public TransformVSCB
{
	con::Buffer m_cbScale;
public:
	TransformScaleVSCB( Graphics &gph, unsigned slot, float scale );

	void accept( IEffectVisitor& ev ) override;
	void bind( Graphics &gph ) cond_noex override;
	std::unique_ptr<IBindableCloning> clone() const noexcept override;
private:
	static con::RawLayout createCbLayout();
};