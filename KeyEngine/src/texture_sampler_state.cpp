#include "texture_sampler_state.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


TextureSamplerState::TextureSamplerState( Graphics &gph,
	const unsigned slot,
	const FilterMode filterMode,
	const AddressMode addressMode )
	:
	m_filteringMode(filterMode),
	m_addressingMode(addressMode),
	m_slot(slot)
{
	D3D11_SAMPLER_DESC samplerDesc{CD3D11_SAMPLER_DESC{CD3D11_DEFAULT{}}};
	//samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = [filterMode]()
	{
		switch( filterMode )
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

	auto checkAddr = [&addressMode] ( )
	{
		return addressMode == AddressMode::Mirror ? D3D11_TEXTURE_ADDRESS_MIRROR :
			( addressMode == AddressMode::Clamp ? D3D11_TEXTURE_ADDRESS_CLAMP :
			( addressMode == AddressMode::Border ? D3D11_TEXTURE_ADDRESS_BORDER :
				D3D11_TEXTURE_ADDRESS_WRAP ) );
	};
	samplerDesc.AddressU = checkAddr();
	samplerDesc.AddressV = checkAddr();
	samplerDesc.AddressW = checkAddr();
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	// lods:
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT hres = getDevice( gph )->CreateSamplerState( &samplerDesc,
		&m_pSamplerState );
	ASSERT_HRES_IF_FAILED;
}

void TextureSamplerState::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetSamplers( m_slot,
		1u,
		m_pSamplerState.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<TextureSamplerState> TextureSamplerState::fetch( Graphics &gph,
	const unsigned slot,
	const FilterMode filt,
	const AddressMode addr )
{
	return BindableMap::fetch<TextureSamplerState>( gph,
		slot,
		filt,
		addr );
}

std::string TextureSamplerState::calcUid( const unsigned slot,
	const FilterMode filt,
	const AddressMode addr )
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
	return typeid( TextureSamplerState ).name()
		+ "@"s
		+ std::to_string( slot )
		+ "#"s
		+ std::to_string( (int)filt )
		+ checkAddr();
}

const std::string TextureSamplerState::getUid() const noexcept
{
	return calcUid( m_slot,
		m_filteringMode,
		m_addressingMode );
}