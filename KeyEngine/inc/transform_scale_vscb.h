#pragma once

#include "transform_vscb.h"
#include "dynamic_constant_buffer.h"


class TransformScaleVSCB
	: public TransformVSCB
{
	con::CBuffer m_cbScale;
public:
	TransformScaleVSCB( Graphics &gph, const unsigned slot, const float scale );
	TransformScaleVSCB( const TransformScaleVSCB &rhs );
	TransformScaleVSCB& operator=( const TransformScaleVSCB &rhs );
	TransformScaleVSCB( TransformScaleVSCB &&rhs ) noexcept;
	TransformScaleVSCB& operator=( TransformScaleVSCB &&rhs ) noexcept;

	void accept( IImGuiEffectVisitor &ev ) override;
	void bind( Graphics &gph ) cond_noex override;
	std::unique_ptr<IBindableCloning> clone() const noexcept override;
	std::unique_ptr<IBindableCloning> clone() noexcept override;
private:
	static con::RawLayout calcCbLayout();
};