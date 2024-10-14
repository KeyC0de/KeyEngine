#include "primitive_topology.h"
#include "graphics.h"
#include "bindable_registry.h"
#include "dxgi_info_queue.h"


// #TODO: add a static function in each bindable that returns a static identifier that identifies the stage of the GPU pipeline this Bindable/GPU-state will run in
// eg PrimitiveTopology runs in Input Assembler

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
	return BindableRegistry::fetch<PrimitiveTopology>( gfx, topology );
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