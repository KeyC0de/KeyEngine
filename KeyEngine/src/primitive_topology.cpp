#include "primitive_topology.h"
#include "bindable_map.h"
#include "dxgi_info_queue.h"


PrimitiveTopology::PrimitiveTopology( Graphics &gph,
	const D3D11_PRIMITIVE_TOPOLOGY topo )
	:
	m_d3dTopo{topo}
{

}

void PrimitiveTopology::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->IASetPrimitiveTopology( m_d3dTopo );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<PrimitiveTopology> PrimitiveTopology::fetch( Graphics &gph,
	const D3D11_PRIMITIVE_TOPOLOGY topo )
{
	return BindableMap::fetch<PrimitiveTopology>( gph,
		topo );
}

std::string PrimitiveTopology::calcUid( const D3D11_PRIMITIVE_TOPOLOGY topo )
{
	using namespace std::string_literals;
	return typeid( PrimitiveTopology ).name() + "#"s + std::to_string( topo );
}

const std::string PrimitiveTopology::getUid() const noexcept
{
	return calcUid( m_d3dTopo );
}