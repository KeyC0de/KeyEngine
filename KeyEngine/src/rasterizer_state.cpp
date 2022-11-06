#include "rasterizer_state.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


RasterizerState::RasterizerState( Graphics &gph,
	const CullMode cullMode,
	const FillMode fillMode /* = Solid */ )
	:
	m_cullMode{cullMode},
	m_fillMode{fillMode}
{
	D3D11_RASTERIZER_DESC rasterDesc{CD3D11_RASTERIZER_DESC{CD3D11_DEFAULT{}}};
	rasterDesc.CullMode = cullMode == CullMode::FrontSided ?
		D3D11_CULL_BACK :
		( cullMode == CullMode::TwoSided ?
			D3D11_CULL_NONE :
			D3D11_CULL_FRONT );
	rasterDesc.FillMode = fillMode == FillMode::Solid ?
		D3D11_FILL_SOLID :
		D3D11_FILL_WIREFRAME;

	HRESULT hres = getDevice( gph )->CreateRasterizerState( &rasterDesc,
		&m_pRasterizerState );
	ASSERT_HRES_IF_FAILED;
}

void RasterizerState::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->RSSetState( m_pRasterizerState.Get() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<RasterizerState> RasterizerState::fetch( Graphics &gph,
	const CullMode cullMode,
	const FillMode fillMode /* = FillMode::Solid */ )
{
	return BindableMap::fetch<RasterizerState>( gph,
		cullMode,
		fillMode );
}

std::string RasterizerState::calcUid( const CullMode cullMode,
	const FillMode fillMode )
{
	using namespace std::string_literals;
	std::string cullModeId;
	std::string fillModeId;
	switch ( cullMode )
	{
	case Solid:
		cullModeId = "S"s;
		break;
	case Wireframe:
		cullModeId = "W"s;
		break;
	}
	switch ( fillMode )
	{
	case FrontSided:
		fillModeId = "FS"s;
		break;
	case BackSided:
		fillModeId = "BS"s;
		break;
	case TwoSided:
		fillModeId = "2S"s;
		break;
	}
	return typeid( RasterizerState ).name()
		+ "#"s
		+ cullModeId
		+ ":"s
		+ fillModeId;
}

const std::string RasterizerState::getUid() const noexcept
{
	return calcUid( m_cullMode,
		m_fillMode );
}

const RasterizerState::CullMode& RasterizerState::getCullMode() const noexcept
{
	return m_cullMode;
}

const RasterizerState::FillMode& RasterizerState::getFillMode() const noexcept
{
	return m_fillMode;
}