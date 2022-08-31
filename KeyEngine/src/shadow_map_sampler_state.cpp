#include "shadow_map_sampler_state.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


ShadowMapSamplerState::ShadowMapSamplerState( Graphics &gph )
{
	for ( unsigned i = 0; i < std::size( m_samplers ); ++i )
	{
		m_samplerMask = i;
		m_samplers[i] = make( gph,
			isTrilinearFiltering(),
			isHwPcfFiltering() );
	}
	setTrilinearFiltering( true );
	setHwPcfFiltering( true );
}

void ShadowMapSamplerState::setTrilinearFiltering( bool bEnable )
{
	m_samplerMask = ( m_samplerMask & ~0b01 ) | ( bEnable ? 0b01 : 0b0 );
}

void ShadowMapSamplerState::setHwPcfFiltering( bool bEnable )
{
	m_samplerMask = ( m_samplerMask & ~0b10 ) | ( bEnable ? 0b10 : 0b0 );
}

bool ShadowMapSamplerState::isTrilinearFiltering() const noexcept
{
	return m_samplerMask & 0b01;
}

bool ShadowMapSamplerState::isHwPcfFiltering() const noexcept
{
	return m_samplerMask & 0b10;
}

unsigned ShadowMapSamplerState::getCurrentSlot() const noexcept
{
	return isHwPcfFiltering() ?
		1 :
		2;
}

size_t ShadowMapSamplerState::getIndex( bool bTrilinear,
	bool bHwPcf ) noexcept
{
	return ( bTrilinear ? 0b01 : 0 ) + ( bHwPcf ? 0b10 : 0 );
}

Microsoft::WRL::ComPtr<ID3D11SamplerState> ShadowMapSamplerState::make( Graphics &gph,
	bool bTrilinear,
	bool bHwPcf )
{
	D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{CD3D11_DEFAULT{}};

	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	if ( bHwPcf )
	{
		samplerDesc.Filter = bTrilinear ?
			D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR :
			D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	}
	else
	{
		samplerDesc.Filter = bTrilinear ?
			D3D11_FILTER_MIN_MAG_MIP_LINEAR :
			D3D11_FILTER_MIN_MAG_MIP_POINT;
	}

	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
	HRESULT hres = getDevice( gph )->CreateSamplerState( &samplerDesc,
		&pSampler );
	ASSERT_HRES_IF_FAILED;
	return std::move( pSampler );
}

void ShadowMapSamplerState::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetSamplers( getCurrentSlot(),
		1,
		m_samplers[m_samplerMask].GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}