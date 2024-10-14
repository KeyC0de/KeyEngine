#include "texture_desc.h"
#include "bindable_exception.h"


const D3D11_TEXTURE2D_DESC createTextureDescriptor( const unsigned width,
	const unsigned height,
	const DXGI_FORMAT format,
	const BindFlags bindFlags,
	const CpuAccessFlags cpuAccessFlags,
	const TextureUsage usage,
	const bool bCube,
	const unsigned nTextures /*= 1u*/,
	const MultisamplingMode multisamplingMode /*= MultisamplingMode::None*/ )
{
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = format;
	texDesc.MipLevels = 1u;
	if ( multisamplingMode == MultisamplingMode::None )
	{
		texDesc.SampleDesc.Count = 1u;
		texDesc.SampleDesc.Quality = 0u;
	}
	else
	{
		//texDesc.SampleDesc.Count = 1u;	// #TODO: multisampling
		//texDesc.SampleDesc.Quality = 0u;
	}
	texDesc.Usage = (D3D11_USAGE) usage;
	texDesc.BindFlags = (unsigned) bindFlags;
	texDesc.CPUAccessFlags = (unsigned) cpuAccessFlags;
	texDesc.ArraySize = ( bCube == true ? 6u : 1u ) * nTextures;
	texDesc.MiscFlags = bCube ?
		D3D11_RESOURCE_MISC_TEXTURECUBE :
		( bindFlags == BindFlags::RenderTargetTexture ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0u );

	return texDesc;
}

const DXGI_FORMAT getFormatRtv( const RenderTargetViewMode mode )
{
	switch ( mode )
	{
	case RenderTargetViewMode::DefaultRT:
		return DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	}
	THROW_BINDABLE_EXCEPTION( "Invalid Render Target View mode DXGI format." );
}

const DXGI_FORMAT getTypelessFormatDsv( const DepthStencilViewMode mode )
{
	switch ( mode )
	{
	case DepthStencilViewMode::DefaultDS:
		return DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;//DXGI_FORMAT_D24_UNORM_S8_UINT
	case DepthStencilViewMode::ShadowDepth:
		return DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
	}
	THROW_BINDABLE_EXCEPTION( "Invalid Depth Stenvil View mode for Typeless DXGI format." );
}

const DXGI_FORMAT getTypedFormatDsv( const DepthStencilViewMode mode )
{
	switch ( mode )
	{
	case DepthStencilViewMode::DefaultDS:
		return DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	case DepthStencilViewMode::ShadowDepth:
		return DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	}
	THROW_BINDABLE_EXCEPTION( "Invalid Depth Stenvil View mode for Typed DXGI format." );
}

const DXGI_FORMAT getShaderInputFormatDsv( const DepthStencilViewMode mode )
{
	switch ( mode )
	{
	case DepthStencilViewMode::DefaultDS:
		return DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case DepthStencilViewMode::ShadowDepth:
		return DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
	}
	THROW_BINDABLE_EXCEPTION( "Invalid Depth Stenvil View mode for Shader Input DXGI format." );
}