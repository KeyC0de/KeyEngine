#include "texture_desc.h"
#include "bindable_exception.h"


const D3D11_TEXTURE2D_DESC createTextureDescriptor( const unsigned width,
	const unsigned height,
	const DXGI_FORMAT format,
	const BindFlags bindFlags,
	const CpuAccessFlags cpuAccessFlags,
	const bool bCube,
	const TextureUsage usage )
{
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = format;
	texDesc.MipLevels = 1u;
	texDesc.SampleDesc.Count = 1u;
	texDesc.SampleDesc.Quality = 0u;
	texDesc.Usage = (D3D11_USAGE) usage;
	texDesc.BindFlags = (unsigned) bindFlags;
	texDesc.CPUAccessFlags = (unsigned) cpuAccessFlags;
	texDesc.ArraySize = bCube ? 6u : 1u;
	texDesc.MiscFlags = bCube ? D3D11_RESOURCE_MISC_TEXTURECUBE :
		( bindFlags == BindFlags::RenderTargetTexture ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0u );

	return texDesc;
}

const DXGI_FORMAT getTypelessFormatDs( const DepthStencilViewMode mode )
{
	switch ( mode )
	{
	case DepthStencilViewMode::Normal:
		return DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;//DXGI_FORMAT_D24_UNORM_S8_UINT
	case DepthStencilViewMode::ShadowDepth:
		return DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
	}
	THROW_BINDABLE_EXCEPTION( "Invalid mode for Typeless DXGI format in IDepthStencilView." );
}

const DXGI_FORMAT getTypedFormatDs( const DepthStencilViewMode mode )
{
	switch ( mode )
	{
	case DepthStencilViewMode::Normal:
		return DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	case DepthStencilViewMode::ShadowDepth:
		return DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	}
	THROW_BINDABLE_EXCEPTION( "Invalid mode for Typed DXGI format in IDepthStencilView." );
}

const DXGI_FORMAT getShaderInputFormatDs( const DepthStencilViewMode mode )
{
	switch ( mode )
	{
	case DepthStencilViewMode::Normal:
		return DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case DepthStencilViewMode::ShadowDepth:
		return DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
	}
	THROW_BINDABLE_EXCEPTION( "Invalid mode for Shader Input DXGI format in IDepthStencilView." );
}