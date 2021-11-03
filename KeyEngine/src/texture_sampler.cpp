#include "texture_sampler.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


TextureSampler::TextureSampler( Graphics& gph,
	unsigned slot,
	Type filterType,
	bool bMirrorTexAddr,
	bool bClampTexAddr )
	:
	m_filterType(filterType),
	m_bMirrorAddr(bMirrorTexAddr),
	m_bClampAddr(bClampTexAddr),
	m_slot(slot)
{
	D3D11_SAMPLER_DESC samplerDesc{CD3D11_SAMPLER_DESC{CD3D11_DEFAULT{}}};
	//samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = [filterType]()
	{
		switch( filterType )
		{
		case Anisotropic:
			return D3D11_FILTER_ANISOTROPIC;
		case Point:
			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case Bilinear:
			return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case Trilinear:
		default:
			return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}
	}();
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.AddressU = bMirrorTexAddr ?
		D3D11_TEXTURE_ADDRESS_MIRROR :
		( bClampTexAddr ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP );
	samplerDesc.AddressV = bMirrorTexAddr ?
		D3D11_TEXTURE_ADDRESS_MIRROR :
		( bClampTexAddr ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP );
	samplerDesc.AddressW = bMirrorTexAddr ?
		D3D11_TEXTURE_ADDRESS_MIRROR :
		( bClampTexAddr ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP );
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

	HRESULT hres = getDevice( gph )->CreateSamplerState( &samplerDesc,
		&m_pSamplerState );
	ASSERT_HRES_IF_FAILED;
}

void TextureSampler::bind( Graphics& gph ) cond_noex
{
	getContext( gph )->PSSetSamplers( m_slot,
		1u,
		m_pSamplerState.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<TextureSampler> TextureSampler::fetch( Graphics& gph,
	unsigned slot,
	Type filterType,
	bool bMirrorTexAddr,
	bool bClampTexAddr )
{
	return BindableMap::fetch<TextureSampler>( gph,
		slot,
		filterType,
		bMirrorTexAddr,
		bClampTexAddr );
}

std::string TextureSampler::generateUID( unsigned slot,
	Type filterType,
	bool bMirrorTexAddr,
	bool bClampTexAddr )
{
	using namespace std::string_literals;
	return typeid( TextureSampler ).name() + "@"s + std::to_string( slot )
		+ "#"s + std::to_string( (int)filterType )
		+ ( bMirrorTexAddr ? "YM"s : "NM"s ) + ( bClampTexAddr ? "YC"s : "NC"s );
}

std::string TextureSampler::getUID() const noexcept
{
	return generateUID( m_slot,
		m_filterType,
		m_bMirrorAddr,
		m_bClampAddr );
}