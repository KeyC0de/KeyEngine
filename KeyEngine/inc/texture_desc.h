#pragma once

#include <d3d11.h>


enum BindFlags : unsigned
{
	TextureOnly = D3D11_BIND_SHADER_RESOURCE,
	RenderTargetOnly = D3D11_BIND_RENDER_TARGET,
	RenderTargetTexture = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
	DepthStencilOnly = D3D11_BIND_DEPTH_STENCIL,
	DepthStencilTexture = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL,
};

enum CpuAccessFlags : unsigned
{
	NoCpuAccess = 0u,
	CpuWriteAccess = D3D11_CPU_ACCESS_WRITE,
	CpuReadAccess = D3D11_CPU_ACCESS_READ,
};

enum TextureUsage : unsigned
{
	Default = D3D11_USAGE_DEFAULT,
	Dynamic = D3D11_USAGE_DYNAMIC,
	Staging = D3D11_USAGE_STAGING,
	Const = D3D11_USAGE_IMMUTABLE,
};

enum RenderTargetViewMode : unsigned
{
	DefaultRT,
};

enum DepthStencilViewMode : unsigned
{
	DefaultDS,
	ShadowDepth,	// for a shadow buffer 32bit float ZB - no SB
};

enum MultisamplingMode : unsigned
{
	None,
	Max,	// #TODO: multisampling
};

const D3D11_TEXTURE2D_DESC createTextureDescriptor( const unsigned width, const unsigned height, const DXGI_FORMAT format, const BindFlags bindFlags, const CpuAccessFlags cpuAccessFlags, const bool bCube, const TextureUsage usage, const MultisamplingMode multisamplingMode = MultisamplingMode::None );

const DXGI_FORMAT getFormatRtv( const RenderTargetViewMode mode );

//	\function		||	\date	2022/10/13 13:52
//	\brief	Use when creating the texture descriptor
//				DS format for D3D11_TEXTURE2D_DESC creation
//				_TYPELESS is actually a float type
//				DXGI_FORMAT_R32_TYPELESS is compatible with both DXGI_FORMAT_D32_FLOAT and DXGI_FORMAT_R32_FLOAT
const DXGI_FORMAT getTypelessFormatDsv( const DepthStencilViewMode mode );

//	\function		||	\date	2022/10/13 13:52
//	\brief	when we want to map a texture view on the Depth Stencil
const DXGI_FORMAT getTypedFormatDsv( const DepthStencilViewMode mode );

//	\function		||	\date	2022/10/13 13:52
//	\brief	when we want to sample from the Depth Stencil in the back buffer
const DXGI_FORMAT getShaderInputFormatDsv( const DepthStencilViewMode mode );