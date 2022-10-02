#include "shadow_map_sampler_state.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "bindable_map.h"


ShadowMapSamplerState::ShadowMapSamplerState( Graphics &gph,
	bool bHwPcf,
	const FilterMode filterMode,
	const unsigned slot_autoCalcedDontSet /*= -1*/ )
	:
	m_bHwPcf{bHwPcf},
	m_filterMode{filterMode},
	m_slot( bHwPcf ? 1 : 2 )
{
	ASSERT( m_slot && m_slot != -1, "Bound slot# is invalid!" );

	D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{CD3D11_DEFAULT{}};

	samplerDesc.BorderColor[0] = 1.0f;	// farthest distance possible -Border color to use if D3D11_TEXTURE_ADDRESS_BORDER is specified for AddressU, AddressV, or AddressW. Range must be between 0.0 and 1.0 inclusive. meaning max distance
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	if ( bHwPcf )
	{
		auto selectFilterMode = [&filterMode] ()
		{
			switch ( filterMode )
			{
			case Linear:
			case Bilinear:
				return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			case Trilinear:
			default:
				return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			}
		};

		samplerDesc.Filter = selectFilterMode();
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	}
	else
	{
		auto selectFilterMode = [&filterMode] ()
		{
			switch ( filterMode )
			{
			case Linear:
				return D3D11_FILTER_MIN_MAG_MIP_POINT;
			case Bilinear:
			case Trilinear:
			default:
				return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			}
		};

		samplerDesc.Filter = selectFilterMode();
	}

	HRESULT hres = getDevice( gph )->CreateSamplerState( &samplerDesc,
		&m_sampler );
	ASSERT_HRES_IF_FAILED;
}

void ShadowMapSamplerState::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetSamplers( m_slot,
		1,
		m_sampler.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

const ShadowMapSamplerState::FilterMode ShadowMapSamplerState::getFilterMode() const noexcept
{
	return m_filterMode;
}

bool ShadowMapSamplerState::isHwPcfFiltering() const noexcept
{
	return m_bHwPcf;
}

const unsigned ShadowMapSamplerState::getSlot() const noexcept
{
	return m_slot;
}

std::shared_ptr<ShadowMapSamplerState> ShadowMapSamplerState::fetch( Graphics &gph,
	const bool bHwPcf,
	const FilterMode filt,
	const unsigned slot )
{
	return BindableMap::fetch<ShadowMapSamplerState>( gph,
		bHwPcf,
		filt,
		slot );
}

std::string ShadowMapSamplerState::calcUid( const bool bHwPcf,
	const FilterMode filterMode,
	const unsigned slot )
{
	using namespace std::string_literals;
	auto checkFilterMode = [&filterMode] ()
	{
		switch ( filterMode )
		{
		case FilterMode::Linear:
			return "LIN"s;
		case FilterMode::Bilinear:
			return "BIL"s;
		case FilterMode::Trilinear:
		default:
			return "TRI"s;
		}
	};

	return typeid( ShadowMapSamplerState ).name()
		+ "@"s
		+ std::to_string( slot )
		+ "#"s
		+ checkFilterMode()
		+ ( bHwPcf ? "PCF"s : ""s );
}

const std::string ShadowMapSamplerState::getUid() const noexcept
{
	return calcUid( m_bHwPcf,
		m_filterMode,
		m_slot );
}