#include "primitive_topology.h"
#include "bindable_map.h"
#include "dxgi_info_queue.h"


PrimitiveTopology::PrimitiveTopology( Graphics &gfx,
	const D3D11_PRIMITIVE_TOPOLOGY topology )
	:
	m_d3dTopology{topology}
{

}

void PrimitiveTopology::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->IASetPrimitiveTopology( m_d3dTopology );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<PrimitiveTopology> PrimitiveTopology::fetch( Graphics &gfx,
	const D3D11_PRIMITIVE_TOPOLOGY topology )
{
	return BindableMap::fetch<PrimitiveTopology>( gfx, topology );
}

std::string PrimitiveTopology::calcUid( const D3D11_PRIMITIVE_TOPOLOGY topology )
{
	using namespace std::string_literals;
	return typeid( PrimitiveTopology ).name() + "#"s + std::to_string( topology );
}

std::string PrimitiveTopology::getUid() const noexcept
{
	return calcUid( m_d3dTopology );
}