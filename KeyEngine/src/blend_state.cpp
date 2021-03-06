#include "blend_state.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"


#define blendStateRTDesc blendStateDesc.RenderTarget[renderTargetSlot]

BlendState::BlendState( Graphics &gph,
	Mode mode,
	unsigned renderTargetSlot,
	std::optional<float> blendFactors )
	:
	m_mode{mode},
	m_renderTargetSlot{renderTargetSlot}
{
	if ( blendFactors )
	{
		m_blendFactors.emplace();
		// set all with the same values for now
		m_blendFactors->fill( *blendFactors );
	}
	else
	{
		m_blendFactors->fill( 0 );
	}

	D3D11_BLEND_DESC blendStateDesc{CD3D11_BLEND_DESC{CD3D11_DEFAULT{}}};
	//default:
	//case NoBlend:
	//blendStateRTDesc.BlendEnable = False;
	//blendStateRTDesc.BlendOp = D3D11_BLEND_OP_ADD;
	//blendStateRTDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	//blendStateRTDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	//blendStateRTDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	//blendStateRTDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	switch ( mode )
	{
	case Additive:
	{
		blendStateRTDesc.BlendEnable = TRUE;
		if ( blendFactors )
		{
			blendStateRTDesc.SrcBlend = D3D11_BLEND_BLEND_FACTOR;
			blendStateRTDesc.DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
		}
		else
		{
			blendStateRTDesc.SrcBlend = D3D11_BLEND_ONE;
			blendStateRTDesc.DestBlend = D3D11_BLEND_ONE;
		}
		break;
	}
	case Multiplicative:
	{
		blendStateRTDesc.BlendEnable = TRUE;
		if ( blendFactors )
		{
			blendStateRTDesc.SrcBlend = D3D11_BLEND_BLEND_FACTOR;
			blendStateRTDesc.DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
		}
		else
		{
			blendStateRTDesc.SrcBlend = D3D11_BLEND_DEST_COLOR;
			blendStateRTDesc.DestBlend = D3D11_BLEND_ZERO;
		}
		break;
	}
	case DoubleMultiplicative:
	{
		blendStateRTDesc.BlendEnable = TRUE;
		if ( blendFactors )
		{
			blendStateRTDesc.SrcBlend = D3D11_BLEND_BLEND_FACTOR;
			blendStateRTDesc.DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
		}
		else
		{
			blendStateRTDesc.SrcBlend = D3D11_BLEND_DEST_COLOR;
			blendStateRTDesc.DestBlend = D3D11_BLEND_SRC_COLOR;
		}
		break;
	}
	case Alpha:
	{
		blendStateRTDesc.BlendEnable = TRUE;
		if ( blendFactors )
		{
			blendStateRTDesc.SrcBlend = D3D11_BLEND_BLEND_FACTOR;
			blendStateRTDesc.DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
		}
		else
		{
			blendStateRTDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendStateRTDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendStateRTDesc.RenderTargetWriteMask = 0x0f;
		}
		break;
	}
	case AlphaToCoverage:
	{
		blendStateDesc.AlphaToCoverageEnable = TRUE;
		break;
	}
	}
	HRESULT hres = getDevice( gph )->CreateBlendState( &blendStateDesc,
		&m_pBlendState );
	ASSERT_HRES_IF_FAILED;
}

void BlendState::bind( Graphics &gph ) cond_noex
{
	getContext( gph )->OMSetBlendState( m_pBlendState.Get(),
		m_blendFactors->data(),
		0xFFFFFFFFu );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<BlendState> BlendState::fetch( Graphics &gph,
	Mode mode,
	unsigned renderTargetSlot,
	std::optional<float> blendFactors )
{
	return BindableMap::fetch<BlendState>( gph,
		mode,
		renderTargetSlot,
		blendFactors );
}

void BlendState::setBlendFactors( float blendFactors ) cond_noex
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	return m_blendFactors->fill( blendFactors );
}

float BlendState::getBlendFactor() const cond_noex
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	// or OMGetBlendState
	return m_blendFactors->front();
}

std::string BlendState::generateUid( Mode mode,
	unsigned renderTargetSlot,
	std::optional<float> blendFactors )
{
	using namespace std::string_literals;
	std::string modeId;
	switch ( mode )
	{
	case Additive:
		modeId = "d"s;
		break;
	case Multiplicative:
		modeId = "m"s;
		break;
	case DoubleMultiplicative:
		modeId = "u"s;
		break;
	case Alpha:
		modeId = "a"s;
		break;
	case AlphaToCoverage:
		modeId = "o"s;
		break;
	case NoBlend:
	default:
		modeId = "n"s;
		break;
	}
	return typeid( BlendState ).name()
		+ "#"s
		+ std::to_string( renderTargetSlot )
		+ "#"s
		+ modeId
		+ ( blendFactors ? "#f"s + std::to_string( *blendFactors ) : ""s );
}

std::string BlendState::getUid() const noexcept
{
	return generateUid( m_mode,
		m_renderTargetSlot,
		m_blendFactors ?
			m_blendFactors->front() :
			std::optional<float>{} );
}
