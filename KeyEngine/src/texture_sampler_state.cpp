#include "texture_sampler_state.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "bindable_exception.h"


TextureSamplerState::TextureSamplerState( Graphics &gph,
	const TextureSamplerMode samplingMode,
	const FilterMode filterMode,
	const AddressMode addressMode )
	:
	m_slot{calcSlot( samplingMode )},
	m_mode{samplingMode},
	m_filteringMode{filterMode},
	m_addressingMode{addressMode == TextureSamplerMode::ShadowTS ? AddressMode::Border : addressMode},
	m_bHwPcf{samplingMode == TextureSamplerMode::ShadowTS ? true : false}
{
	D3D11_SAMPLER_DESC samplerDesc{CD3D11_SAMPLER_DESC{CD3D11_DEFAULT{}}};
	// Defaults:
	//.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	// filtering method to use when sampling a texture
	//.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;		// method to use for resolving a texture coordinate (u, v, or w) that is outside the 0 to 1 range
	//.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	//.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	//.MipLODBias = 0.0f;		// offset from the calculated mipmap level. For example, if Direct3D calculates that a texture should be sampled at mipmap level 3 and MipLODBias is 2, then the texture will be sampled at mipmap level 5.
	//.MaxAnisotropy = 1;		// clamping value used if D3D11_FILTER_ANISOTROPIC or D3D11_FILTER_COMPARISON_ANISOTROPIC is specified in Filter. Valid values are between 1 and 16 (D3D11_REQ_MAXANISOTROPY).
	//.ComparisonFunc = D3D11_COMPARISON_NEVER;		// a function that compares sampled data (src) against existing sampled data (dest)
	//												// with D3D11_COMPARISON_NEVER the comparison never passes.
	//.BorderColor[0] = 1.0f;	// max/farthest distance possible - color to use for borders if D3D11_TEXTURE_ADDRESS_BORDER is specified for AddressU, AddressV, or AddressW. Range must be between 0.0 and 1.0 inclusive.
	//.BorderColor[1] = 1.0f;
	//.BorderColor[2] = 1.0f;
	//.BorderColor[3] = 1.0f;
	//.MinLOD = -3.402823466e+38F;	// -FLT_MAX or -D3D11_FLOAT32_MAX, specifies the lower end of the mipmap range to clamp access to, where 0 is the largest and most detailed mipmap level and any level higher than that is less detailed
	//.MaxLOD = 3.402823466e+38F;	// FLT_MAX or D3D11_FLOAT32_MAX

	if ( samplingMode == TextureSamplerMode::DefaultTS )
	{
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

		auto checkAddr = [addressMode] ()
		{
			return addressMode == AddressMode::Mirror ? D3D11_TEXTURE_ADDRESS_MIRROR :
				( addressMode == AddressMode::Clamp ? D3D11_TEXTURE_ADDRESS_CLAMP :
				( addressMode == AddressMode::Border ? D3D11_TEXTURE_ADDRESS_BORDER : D3D11_TEXTURE_ADDRESS_WRAP ) );
		};
		samplerDesc.AddressU = checkAddr();
		samplerDesc.AddressV = checkAddr();
		samplerDesc.AddressW = checkAddr();
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		samplerDesc.MinLOD = 0.0f;
	}
	else if ( samplingMode == TextureSamplerMode::ShadowTS )
	{
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		if ( m_bHwPcf )
		{
			auto selectFilterMode = [filterMode] ()
			{
				switch ( filterMode )
				{
				case Point:
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
			auto selectFilterMode = [filterMode] ()
			{
				switch ( filterMode )
				{
				case Point:
					return D3D11_FILTER_MIN_MAG_MIP_POINT;
				case Bilinear:
				case Trilinear:
				default:
					return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				}
			};

			samplerDesc.Filter = selectFilterMode();
		}
	}
	else
	{
		THROW_BINDABLE_EXCEPTION( "Invalid Texture Sampler mode." );
	}

	HRESULT hres = getDevice( gph )->CreateSamplerState( &samplerDesc, &m_pSamplerState );
	ASSERT_HRES_IF_FAILED;
}

void TextureSamplerState::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetSamplers( m_slot, 1u, m_pSamplerState.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<TextureSamplerState> TextureSamplerState::fetch( Graphics &gph,
	const TextureSamplerMode samplingMode,
	const FilterMode filterMode,
	const AddressMode addressMode )
{
	return BindableMap::fetch<TextureSamplerState>( gph, samplingMode, filterMode, addressMode );
}

std::string TextureSamplerState::calcUid( const TextureSamplerMode samplingMode,
	const FilterMode filterMode,
	const AddressMode addressMode )
{
	auto getSamplingModeStr = [&samplingMode] ()
	{
		switch ( samplingMode )
		{
		default:
		case TextureSamplerState::DefaultTS:
			return "Def";
		case TextureSamplerState::ShadowTS:
			return "Sha";
		}
	};

	auto getAddressModeStr = [&addressMode] ()
	{
		switch ( addressMode )
		{
		case AddressMode::Mirror:
			return "MIR";
		case AddressMode::Clamp:
			return "CLM";
		case AddressMode::Border:
			return "BRD";
		case AddressMode::Wrap:
		default:
			return "WRP";
		}
	};

	using namespace std::string_literals;
	return typeid( TextureSamplerState ).name() + "#"s + std::to_string( calcSlot( samplingMode ) ) + ":"s + getSamplingModeStr() + std::to_string( (int)filterMode ) + ":"s + getAddressModeStr() + ":"s + std::to_string( samplingMode == TextureSamplerMode::ShadowTS ? true : false );
}

std::string TextureSamplerState::getUid() const noexcept
{
	return calcUid( m_mode, m_filteringMode, m_addressingMode );
}

unsigned TextureSamplerState::calcSlot( const TextureSamplerMode samplingMode ) noexcept
{
	switch ( samplingMode )
	{
	default:
	case TextureSamplerMode::DefaultTS:
		return 0u;
	case TextureSamplerMode::ShadowTS:
//		{
//			if ( bHwPcf )
//			{
				return 1u;
//			}
//			else
//			{
//				return 2u;
//			}
//		}
	}
}

unsigned TextureSamplerState::getSlot() const noexcept
{
	return m_slot;
}