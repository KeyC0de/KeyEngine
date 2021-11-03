#include "depth_stencil_state.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


DepthStencilState::DepthStencilState( Graphics& gph,
	Mode mode )
	:
	m_mode(mode)
{
	D3D11_DEPTH_STENCIL_DESC dsDesc{CD3D11_DEPTH_STENCIL_DESC{CD3D11_DEFAULT{}}};
	// Default:
	//DepthEnable = TRUE;
	//DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//DepthFunc = D3D11_COMPARISON_LESS;
	//StencilEnable = FALSE;
	//StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	//StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	//defaultStencilOp is D3D11_STENCIL_OP_KEEP
	//FrontFace = defaultStencilOp;
	//BackFace = defaultStencilOp;
	if ( mode == DepthOffStencilOff )
	{
		dsDesc.DepthEnable = FALSE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	}
	else if ( mode == DepthOnStencilWriteFF )
	{
		dsDesc.StencilEnable = TRUE;
		dsDesc.StencilWriteMask = 0xFFui8;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	}
	else if ( mode == DepthOffStencilWriteFF )
	{
		dsDesc.DepthEnable = FALSE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.StencilEnable = TRUE;
		dsDesc.StencilWriteMask = 0xFFui8;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	}
	else if ( mode == DepthOffStencilReadFF )
	{
		dsDesc.DepthEnable = FALSE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.StencilEnable = TRUE;
		dsDesc.StencilReadMask = 0xFFui8;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	}
	else if ( mode == DepthReversed )
	{
		dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;
	}
	else if ( mode == DepthEquals1 )
	{
		dsDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	}

	HRESULT hres = getDevice( gph )->CreateDepthStencilState( &dsDesc,
		&m_pDepthStencilState );
	ASSERT_HRES_IF_FAILED;
}

void DepthStencilState::bind( Graphics& gph ) cond_noex
{
	getContext( gph )->OMSetDepthStencilState( m_pDepthStencilState.Get(),
		0xFF );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<DepthStencilState> DepthStencilState::fetch( Graphics& gph,
	Mode mode )
{
	return BindableMap::fetch<DepthStencilState>( gph,
		mode );
}

std::string DepthStencilState::generateUID( Mode mode )
{
	using namespace std::string_literals;
	const auto getModeStr = [mode]()
	{
		switch( mode )
		{
		case Default:
			return "Default"s;
		case DepthOffStencilWriteFF:
			return "DepthOffStencilWriteFF"s;
		case DepthOffStencilReadFF:
			return "DepthOffStencilReadFF"s;
		case DepthOffStencilOff:
			return "DepthOffStencilOff"s;
		case DepthReversed:
			return "DepthReversed"s;
		case DepthEquals1:
			return "DepthEquals1"s;
		default:
			return "!INV!"s;
		}
	};
	return typeid( DepthStencilState ).name() + "#"s + getModeStr();
}

std::string DepthStencilState::getUID() const noexcept
{
	return generateUID( m_mode );
}