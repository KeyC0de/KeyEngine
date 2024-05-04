#pragma once

#include "bindable.h"


class PrimitiveTopology
	: public IBindable
{
protected:
	D3D11_PRIMITIVE_TOPOLOGY m_d3dTopology;
public:
	PrimitiveTopology( Graphics &gfx, const D3D11_PRIMITIVE_TOPOLOGY topology );

	void bind( Graphics &gfx ) cond_noex override;
	static std::shared_ptr<PrimitiveTopology> fetch( Graphics &gfx, const D3D11_PRIMITIVE_TOPOLOGY topology );
	static std::string calcUid( const D3D11_PRIMITIVE_TOPOLOGY topology );
	std::string getUid() const noexcept override;
};