#pragma once

#include "bindable.h"


class PrimitiveTopology
	: public IBindable
{
protected:
	D3D11_PRIMITIVE_TOPOLOGY m_d3dPrimTop;
public:
	PrimitiveTopology( Graphics& gph, D3D11_PRIMITIVE_TOPOLOGY topo );

	void bind( Graphics& gph ) cond_noex override;
	static std::shared_ptr<PrimitiveTopology> fetch( Graphics& gph,
		D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	static std::string generateUid( D3D11_PRIMITIVE_TOPOLOGY topo );
	std::string getUid() const noexcept override;
};