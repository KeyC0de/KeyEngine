#include "rasterizer.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


Rasterizer::Rasterizer( Graphics& gph,
	bool bTwoSided,
	bool bWireframe )
	:
	m_bTwoSided{bTwoSided},
	m_bWireframe{bWireframe}
{
	D3D11_RASTERIZER_DESC rasterDesc{CD3D11_RASTERIZER_DESC{CD3D11_DEFAULT{}}};
	rasterDesc.FillMode = bWireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	rasterDesc.CullMode = bTwoSided ?
		D3D11_CULL_NONE :
		D3D11_CULL_BACK;

	HRESULT hres = getDevice( gph )->CreateRasterizerState( &rasterDesc,
		&m_pRasterizerState );
	ASSERT_HRES_IF_FAILED;
}

void Rasterizer::bind( Graphics& gph ) cond_noex
{
	getContext( gph )->RSSetState( m_pRasterizerState.Get() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<Rasterizer> Rasterizer::fetch( Graphics& gph,
	bool bTwoSided,
	bool bWireframe )
{
	return BindableMap::fetch<Rasterizer>( gph,
		bTwoSided,
		bWireframe );
}

std::string Rasterizer::generateUID( bool bTwoSided,
	bool bWireframe )
{
	using namespace std::string_literals;
	return typeid( Rasterizer ).name() + "#"s + ( bTwoSided ? "2s"s : "1s"s )
		+ ( bWireframe ? "W"s : "S"s );
}

std::string Rasterizer::getUID() const noexcept
{
	return generateUID( m_bTwoSided,
		m_bWireframe );
}