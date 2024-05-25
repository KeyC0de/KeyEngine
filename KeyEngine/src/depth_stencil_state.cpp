#include "depth_stencil_state.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "bindable_exception.h"


DepthStencilState::DepthStencilState( Graphics &gfx,
	const Mode mode )
	:
	m_mode(mode)
{
	D3D11_DEPTH_STENCIL_DESC dsDesc{CD3D11_DEPTH_STENCIL_DESC{CD3D11_DEFAULT{}}};
	// if ( mode == Default )
	//.DepthEnable = TRUE;							// enable depth testing by default
	//.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;	// identify the portion of a depth-stencil buffer for writing depth data, basically whether to enable depth-stencil buffer; D3D11_DEPTH_WRITE_MASK_ALL turns on writes to the D.S.B, D3D11_DEPTH_WRITE_MASK_ZERO turns off writes to the D.S.B.
	//.DepthFunc = D3D11_COMPARISON_LESS;			// a function that compares depth data against existing depth data
	//.StencilEnable = FALSE;						// disable stencil testing by default
	//.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;	// identify a portion of the depth-stencil buffer for writing stencil data (up to 0xFF)
	//.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;	// identify a portion of the depth-stencil buffer for reading stencil data (up to 0xFF)
	//.FrontFace = defaultStencilOp;				// identify how to use the results of the depth test and the stencil test for pixels whose surface normal is facing towards the camera
	//.BackFace = defaultStencilOp;					// identify how to use the results of the depth test and the stencil test for pixels whose surface normal is facing away from the camera
	//Front|BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;		// a function that compares stencil data against existing stencil data; D3D11_COMPARISON_ALWAYS always passes the comparison
	//							with D3D11_COMPARISON_NOT_EQUAL the comparison passes if the source data is not equal to the destination data
	//Front|BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;		// set the stencil operation to be performed when both depth & stencil tests pass (if ZB is disabled while SB is enabled and S.T. passes then this operation is also considered a pass); D3D11_STENCIL_OP_KEEP keeps the existing stencil data
	//Front|BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;		// the stencil operation to perform when stencil testing fails
	//Front|BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;// the stencil operation to perform when stencil testing passes and depth testing fails
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
	else if ( mode == DepthOnStencilReadFF )
	{
		dsDesc.StencilEnable = TRUE;
		dsDesc.StencilReadMask = 0xFFui8;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
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
	else if ( mode == DepthOnStencilWriteBF )
	{
		dsDesc.StencilEnable = TRUE;
		dsDesc.StencilWriteMask = 0xFFui8;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	}
	else if ( mode == DepthOnStencilReadBF )
	{
		dsDesc.StencilEnable = TRUE;
		dsDesc.StencilReadMask = 0xFFui8;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	}
	else if ( mode == DepthOffStencilWriteBF )
	{
		dsDesc.DepthEnable = FALSE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.StencilEnable = TRUE;
		dsDesc.StencilWriteMask = 0xFFui8;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	}
	else if ( mode == DepthOffStencilReadBF )
	{
		dsDesc.DepthEnable = FALSE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.StencilEnable = TRUE;
		dsDesc.StencilReadMask = 0xFFui8;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	}
	else if ( mode == DepthReversedStencilOff )
	{
		dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;
	}
	else if ( mode == DepthReadOnlyStencilOff )
	{
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	}
	else if ( mode == DepthReadOnlyEquals1StencilOff )
	{
		dsDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	}
	else if ( mode != Default )
	{
		THROW_BINDABLE_EXCEPTION( "Invalid Depth Stencil mode." );
	}

	HRESULT hres = getDevice( gfx )->CreateDepthStencilState( &dsDesc, &m_pDepthStencilState );
	ASSERT_HRES_IF_FAILED;
}

void DepthStencilState::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->OMSetDepthStencilState( m_pDepthStencilState.Get(), 0xFF );
	DXGI_GET_QUEUE_INFO( gfx );
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& DepthStencilState::getD3dDepthStencilState()
{
	return m_pDepthStencilState;
}

std::shared_ptr<DepthStencilState> DepthStencilState::fetch( Graphics &gfx,
	const Mode mode )
{
	return BindableMap::fetch<DepthStencilState>( gfx, mode );
}

std::string DepthStencilState::calcUid( const Mode mode )
{
	using namespace std::string_literals;
	const auto getModeStr = [mode]()
	{
		switch( mode )
		{
		case Default:
			return "Default"s;
		case DepthOffStencilOff:
			return "DepthOffStencilOff"s;
		case DepthOnStencilWriteFF:
			return "DepthOnStencilWriteFF"s;
		case DepthOffStencilWriteFF:
			return "DepthOffStencilWriteFF"s;
		case DepthOnStencilReadFF:
			return "DepthOnStencilReadFF"s;
		case DepthOffStencilReadFF:
			return "DepthOffStencilReadFF"s;
		case DepthOnStencilWriteBF:
			return "DepthOnStencilWriteBF"s;
		case DepthOffStencilWriteBF:
			return "DepthOffStencilWriteBF"s;
		case DepthOnStencilReadBF:
			return "DepthOnStencilReadBF"s;
		case DepthOffStencilReadBF:
			return "DepthOffStencilReadBF"s;
		case DepthReversedStencilOff:
			return "DepthReversedStencilOff"s;
		case DepthReadOnlyEquals1StencilOff:
			return "DepthReadOnlyEquals1StencilOff"s;
		default:
			return "!INV!"s;
		}
	};
	return typeid( DepthStencilState ).name() + "#"s + getModeStr();
}

std::string DepthStencilState::getUid() const noexcept
{
	return calcUid( m_mode );
}