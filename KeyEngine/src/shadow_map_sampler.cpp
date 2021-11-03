#include "shadow_map_sampler.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


ShadowMapSampler::ShadowMapSampler( Graphics& gph )
{
	for ( unsigned i = 0; i < 4; ++i )
	{
		m_samplerMask = i;
		m_samplers[i] = make( gph,
			isTrilinearFiltering(),
			isHwPcfFiltering() );
	}
	setTrilinearFiltering( true );
	setHwPcfFiltering( true );
}

void ShadowMapSampler::setTrilinearFiltering( bool bEnable )
{
	m_samplerMask = ( m_samplerMask & ~0b01 ) | ( bEnable ? 0b01 : 0b0 );
}

void ShadowMapSampler::setHwPcfFiltering( bool bEnable )
{
	m_samplerMask = ( m_samplerMask & ~0b10 ) | ( bEnable ? 0b10 : 0b0 );
}

bool ShadowMapSampler::isTrilinearFiltering() const
{
	return m_samplerMask & 0b01;
}

bool ShadowMapSampler::isHwPcfFiltering() const
{
	return m_samplerMask & 0b10;
}

unsigned ShadowMapSampler::getCurrentSlot() const
{
	return isHwPcfFiltering() ?
		1 :
		2;	// Trilinear
}

size_t ShadowMapSampler::getIndex( bool bTrilinear,
	bool bHwPcf )
{
	return ( bTrilinear ? 0b01 : 0 ) + ( bHwPcf ? 0b10 : 0 );
}

Microsoft::WRL::ComPtr<ID3D11SamplerState> ShadowMapSampler::make( Graphics& gph,
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

void ShadowMapSampler::bind( Graphics& gph ) cond_noex
{
	getContext( gph )->PSSetSamplers( getCurrentSlot(),
		1,
		m_samplers[m_samplerMask].GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}