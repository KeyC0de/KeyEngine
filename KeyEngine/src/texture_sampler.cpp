#include "texture_sampler.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


TextureSampler::TextureSampler( Graphics &gph,
	unsigned slot,
	FilterMode filt,
	AddressMode addr )
	:
	m_filtering(filt),
	m_addressing(addr),
	m_slot(slot)
{
	D3D11_SAMPLER_DESC samplerDesc{CD3D11_SAMPLER_DESC{CD3D11_DEFAULT{}}};
	//samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = [filt]()
	{
		switch( filt )
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
	auto checkAddr = [&addr] ( )
	{
		return addr == AddressMode::Mirror ?
			D3D11_TEXTURE_ADDRESS_MIRROR :
			( addr == AddressMode::Clamp ? D3D11_TEXTURE_ADDRESS_CLAMP :
			( addr == AddressMode::Border ? D3D11_TEXTURE_ADDRESS_BORDER : D3D11_TEXTURE_ADDRESS_WRAP ) );
	};
	samplerDesc.AddressU = checkAddr();
	samplerDesc.AddressV = checkAddr();
	samplerDesc.AddressW = checkAddr();
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

	HRESULT hres = getDevice( gph )->CreateSamplerState( &samplerDesc,
		&m_pSamplerState );
	ASSERT_HRES_IF_FAILED;
}

void TextureSampler::bind( Graphics &gph ) cond_noex
{
	getContext( gph )->PSSetSamplers( m_slot,
		1u,
		m_pSamplerState.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<TextureSampler> TextureSampler::fetch( Graphics &gph,
	unsigned slot,
	FilterMode filt,
	AddressMode addr )
{
	return BindableMap::fetch<TextureSampler>( gph,
		slot,
		filt,
		addr );
}

std::string TextureSampler::generateUid( unsigned slot,
	FilterMode filt,
	AddressMode addr )
{
	auto checkAddr = [&addr] ()
	{
		switch ( addr )
		{
		case AddressMode::Mirror:
			return "MIR";
		case AddressMode::Clamp:
			return "CLM";
		case AddressMode::Border:
			return "BRD";
		default:
			return "WRP";
		}
	};

	using namespace std::string_literals;
	return typeid( TextureSampler ).name() + "@"s + std::to_string( slot )
		+ "#"s + std::to_string( (int)filt )
		+ checkAddr();
}

std::string TextureSampler::getUid() const noexcept
{
	return generateUid( m_slot,
		m_filtering,
		m_addressing );
}