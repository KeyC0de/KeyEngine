#pragma once

#include "bindable.h"


class PrimitiveTopology
	: public IBindable
{
protected:
	D3D11_PRIMITIVE_TOPOLOGY m_d3dTopology;
public:
	PrimitiveTopology( Graphics &gph, const D3D11_PRIMITIVE_TOPOLOGY topology );

	void bind( Graphics &gph ) cond_noex override;
	static std::shared_ptr<PrimitiveTopology> fetch( Graphics &gph, const D3D11_PRIMITIVE_TOPOLOGY topology );
	static std::string calcUid( const D3D11_PRIMITIVE_TOPOLOGY topology );
	std::string getUid() const noexcept override;
};