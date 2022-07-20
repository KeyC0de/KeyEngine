#include "primitive_topology.h"
#include "bindable_map.h"


PrimitiveTopology::PrimitiveTopology( Graphics &gph,
	D3D11_PRIMITIVE_TOPOLOGY topo )
	:
	m_d3dPrimTop{topo}
{

}

void PrimitiveTopology::bind( Graphics &gph ) cond_noex
{
	getContext( gph )->IASetPrimitiveTopology( m_d3dPrimTop );
}

std::shared_ptr<PrimitiveTopology> PrimitiveTopology::fetch( Graphics &gph,
	D3D11_PRIMITIVE_TOPOLOGY topo )
{
	return BindableMap::fetch<PrimitiveTopology>( gph,
		topo );
}

std::string PrimitiveTopology::generateUid( D3D11_PRIMITIVE_TOPOLOGY topo )
{
	using namespace std::string_literals;
	return typeid( PrimitiveTopology ).name() + "#"s + std::to_string( topo );
}

std::string PrimitiveTopology::getUid() const noexcept
{
	return generateUid( m_d3dPrimTop );
}