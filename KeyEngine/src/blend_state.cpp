#include "blend_state.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"


#define blendStateDescRTSlot blendStateDesc.RenderTarget[renderTargetSlot]

BlendState::BlendState( Graphics &gfx,
	const Mode mode,
	const unsigned renderTargetSlot,
	const std::optional<float> blendFactors /*= {}*/,
	const unsigned multisampleMask /*= 0xFFFFFFFFu*/ )
	:
	m_mode{mode},
	m_renderTargetSlot{renderTargetSlot},
	m_multisampleMask{multisampleMask}
{
	D3D11_BLEND_DESC blendStateDesc{CD3D11_BLEND_DESC{CD3D11_DEFAULT{}}};
	//default case NoBlend:
	//blendStateDesc.AlphaToCoverageEnable = FALSE;
	//blendStateDesc.IndependentBlendEnable = FALSE;
	//blendStateDescRTSlot.BlendEnable = False;
	//blendStateDescRTSlot.SrcBlend = D3D11_BLEND_ONE;
	//blendStateDescRTSlot.DestBlend = D3D11_BLEND_ZERO;
	//blendStateDescRTSlot.BlendOp = D3D11_BLEND_OP_ADD;
	//blendStateDescRTSlot.SrcBlendAlpha = D3D11_BLEND_ONE;		// whether to blend the alpha channel values as well
	//blendStateDescRTSlot.DestBlendAlpha = D3D11_BLEND_ZERO;	// otherwise only blend the RGB channel values
	//blendStateDescRTSlot.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	//blendStateDescRTSlot.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;	// per-bit targeting of specific channels to blend

	if ( blendFactors )
	{
		m_blendFactors.emplace();
		m_blendFactors->fill( *blendFactors );	// set all channels with the same value for now

		blendStateDescRTSlot.BlendEnable = TRUE;
		blendStateDescRTSlot.SrcBlend = D3D11_BLEND_BLEND_FACTOR;
		blendStateDescRTSlot.DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
	}
	else
	{
		std::array<float, 4> emptyArray{0};
		emptyArray.fill( 1.0f );
		m_blendFactors = std::make_optional<std::array<float, 4>>( emptyArray );

		switch ( mode )
		{
		case Additive:
		{
			blendStateDescRTSlot.BlendEnable = TRUE;
			blendStateDescRTSlot.SrcBlend = D3D11_BLEND_ONE;
			blendStateDescRTSlot.DestBlend = D3D11_BLEND_ONE;
			break;
		}
		case Multiplicative:
		{
			blendStateDescRTSlot.BlendEnable = TRUE;
			blendStateDescRTSlot.SrcBlend = D3D11_BLEND_DEST_COLOR;
			blendStateDescRTSlot.DestBlend = D3D11_BLEND_ZERO;
			break;
		}
		case DoubleMultiplicative:
		{
			blendStateDescRTSlot.BlendEnable = TRUE;
			blendStateDescRTSlot.SrcBlend = D3D11_BLEND_DEST_COLOR;
			blendStateDescRTSlot.DestBlend = D3D11_BLEND_SRC_COLOR;
			break;
		}
		case Alpha:
		{
			blendStateDescRTSlot.BlendEnable = TRUE;
			blendStateDescRTSlot.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendStateDescRTSlot.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendStateDescRTSlot.SrcBlendAlpha = D3D11_BLEND_ZERO;
			blendStateDescRTSlot.RenderTargetWriteMask = 0x0f;
			break;
		}
		case AlphaToCoverage:
		{
			blendStateDesc.AlphaToCoverageEnable = TRUE;
			break;
		}
		}
	}

	HRESULT hres = getDevice( gfx )->CreateBlendState( &blendStateDesc, &m_pD3dBlendState );
	ASSERT_HRES_IF_FAILED;
}

void BlendState::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->OMSetBlendState( m_pD3dBlendState.Get(), m_blendFactors->data(), m_multisampleMask );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<BlendState> BlendState::fetch( Graphics &gfx,
	const Mode mode,
	const unsigned renderTargetSlot,
	std::optional<float> blendFactors /*= {}*/,
	const unsigned multisampleMask /*= 0xFFFFFFFFu*/ )
{
	return BindableMap::fetch<BlendState>( gfx, mode, renderTargetSlot, blendFactors );
}

std::string BlendState::calcUid( const Mode mode,
	const unsigned renderTargetSlot,
	std::optional<float> blendFactors /*= {}*/,
	const unsigned multisampleMask /*= 0xFFFFFFFFu*/ )
{
	using namespace std::string_literals;
	std::string modeId;
	switch ( mode )
	{
	case Additive:
		modeId = "Add"s;
		break;
	case Multiplicative:
		modeId = "Mul"s;
		break;
	case DoubleMultiplicative:
		modeId = "Dmu"s;
		break;
	case Alpha:
		modeId = "Alp"s;
		break;
	case AlphaToCoverage:
		modeId = "Alc"s;
		break;
	case NoBlend:
	default:
		modeId = "Non"s;
		break;
	}
	return typeid( BlendState ).name() + "#"s + std::to_string( renderTargetSlot ) + ":"s + modeId + ( blendFactors ? "#f"s + std::to_string( *blendFactors ) : ""s );
}

std::string BlendState::getUid() const noexcept
{
	return calcUid( m_mode, m_renderTargetSlot, m_blendFactors ? m_blendFactors->front() : std::optional<float>{} );
}

void BlendState::fillBlendFactors( const float sameBlendFactor ) cond_noex
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	m_blendFactors->fill( sameBlendFactor );
}

void BlendState::setBlendFactors( std::array<float, 4> blendFactors ) cond_noex
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	m_blendFactors->swap( blendFactors );
}

void BlendState::setBlendFactorRed( const float blendFactorRed ) cond_noex
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	( *m_blendFactors )[0] = blendFactorRed;
}

void BlendState::setBlendFactorGreen( const float blendFactorGreen ) cond_noex
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	( *m_blendFactors )[1] = blendFactorGreen;
}

void BlendState::setBlendFactorBlue( const float blendFactorBlue ) cond_noex
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	( *m_blendFactors )[2] = blendFactorBlue;
}

void BlendState::setBlendFactorAlpha( const float blendFactorAlpha ) cond_noex
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	( *m_blendFactors )[3] = blendFactorAlpha;
}

float BlendState::getBlendFactorRed() const noexcept
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	return m_blendFactors.value()[0];
}

float BlendState::getBlendFactorGreen() const noexcept
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	return m_blendFactors.value()[1];
}

float BlendState::getBlendFactorBlue() const noexcept
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	return m_blendFactors.value()[2];
}

float BlendState::getBlendFactorAlpha() const noexcept
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	return m_blendFactors.value()[3];
}

const std::array<float, 4>& BlendState::getBlendFactors() const noexcept
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	return *m_blendFactors;
}

std::array<float, 4>& BlendState::blendFactors()
{
	ASSERT( m_blendFactors, "No blend factors set!" );
	return *m_blendFactors;
}

const BlendState::Mode& BlendState::getBlendMode() const noexcept
{
	return m_mode;
}

Microsoft::WRL::ComPtr<ID3D11BlendState>& BlendState::getD3dBlendState()
{
	return m_pD3dBlendState;
}
