#pragma once

#include "winner.h"
#include <string>
#include <vector>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "assertions_console.h"
#include "os_utils.h"
#include "key_wrl.h"


namespace util
{

DirectX::XMFLOAT3 extractEulerAngles( const DirectX::XMFLOAT4X4& mat );
DirectX::XMFLOAT3 extractTranslation( const DirectX::XMFLOAT4X4& mat );
DirectX::XMMATRIX scaleTranslation( const DirectX::XMMATRIX &mat, float scale );

//===================================================
//	\function	comSafeRelease
//	\brief  safely release a COM object
//	\date	2018/12/30 17:35
template<typename T>
inline void comSafeRelease( T& p )
{
	if ( p != nullptr )
	{
		p->Release();
		p = nullptr;
	}
}

DXGI_RATIONAL queryRefreshRate( unsigned screenWidth, unsigned screenHeight, bool bVsync );

//===================================================
//	\function	loadShaderDynamically
//	\brief  will be used to load and compile a shader at runtime
//	\date	2019/12/30 17:28
template<class TShader>
static TShader* createShaderObject( ID3D11Device* pD3dDevice,
	ID3DBlob*& pShaderBlob,
	ID3D11ClassLinkage* pClassLinkage );

template<>
static ID3D11VertexShader* createShaderObject<ID3D11VertexShader>( ID3D11Device* pD3dDevice,
	ID3DBlob*& pShaderBlob,
	ID3D11ClassLinkage* pClassLinkage )
{
	ASSERT( pD3dDevice, "d3d11 Device is null!" );
	ASSERT( pShaderBlob, "d3d11 shader object is null!" );

	ID3D11VertexShader* pVertexShader;
	HRESULT hres = pD3dDevice->CreateVertexShader( pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(),
		pClassLinkage,
		&pVertexShader );
	ASSERT_HRES_IF_FAILED;

	return pVertexShader;
}

template<>
static ID3D11PixelShader* createShaderObject<ID3D11PixelShader>( ID3D11Device* pD3dDevice,
	ID3DBlob*& pShaderBlob,
	ID3D11ClassLinkage* pClassLinkage )
{
	ASSERT( pD3dDevice, "d3d11 Device is null!" );
	ASSERT( pShaderBlob, "d3d11 shader object is null!" );

	ID3D11PixelShader* pPixelShader;
	HRESULT hres = pD3dDevice->CreatePixelShader( pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(),
		pClassLinkage,
		&pPixelShader );
	ASSERT_HRES_IF_FAILED;

	return pPixelShader;
}

template<class TShader>
TShader* loadShaderDynamically( ID3D11Device* pD3dDevice,
	const std::wstring &fileName,
	const std::string &entryPoint,
	const std::string &profile,
	ID3DBlob*& pShaderBlob,
	const std::vector<D3D_SHADER_MACRO>& defines = {} )
{
	Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;

	const auto queryFeatureLevel = [&profile, &pD3dDevice]() -> std::string
	{
		if ( profile == "latest" )
		{
			D3D_FEATURE_LEVEL featureLevel = pD3dDevice->GetFeatureLevel();
			if constexpr( std::is_same_v<TShader, ID3D11VertexShader> )
			{
				switch ( featureLevel )
				{
					case D3D_FEATURE_LEVEL_11_1:
					case D3D_FEATURE_LEVEL_11_0:
					{
						return "vs_5_0";
						break;
					}
					case D3D_FEATURE_LEVEL_10_1:
					{
						return "vs_4_1";
						break;
					}
					case D3D_FEATURE_LEVEL_10_0:
					{
						return "vs_4_0";
						break;
					}
					case D3D_FEATURE_LEVEL_9_3:
					{
						return "vs_4_0_level_9_3";
						break;
					}
					case D3D_FEATURE_LEVEL_9_2:
						[[fallthrough]];
					case D3D_FEATURE_LEVEL_9_1:
					{
						return "vs_4_0_level_9_1";
						break;
					}
				}
			}
			else if constexpr ( std::is_same_v<TShader, ID3D11PixelShader> )
			{
				switch ( featureLevel )
				{
					case D3D_FEATURE_LEVEL_11_1:
					case D3D_FEATURE_LEVEL_11_0:
					{
						return "ps_5_0";
						break;
					}
					case D3D_FEATURE_LEVEL_10_1:
					{
						return "ps_4_1";
						break;
					}
					case D3D_FEATURE_LEVEL_10_0:
					{
						return "ps_4_0";
						break;
					}
					case D3D_FEATURE_LEVEL_9_3:
					{
						return "ps_4_0_level_9_3";
						break;
					}
					case D3D_FEATURE_LEVEL_9_2:
						[[fallthrough]];
					case D3D_FEATURE_LEVEL_9_1:
					{
						return "ps_4_0_level_9_1";
						break;
					}
				}
			}
		}
		return "";
	};

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined _DEBUG && !defined NDEBUG
	flags |= D3DCOMPILE_DEBUG;
#endif

	HRESULT hres = D3DCompileFromFile( fileName.data(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint.c_str(),
		queryFeatureLevel().c_str(),
		flags,
		0u,
		&pShaderBlob,
		&pErrorBlob );
	if ( pErrorBlob )
	{
		OutputDebugStringW( (LPCWSTR) pErrorBlob->GetBufferPointer() );
	}
	ASSERT_HRES_IF_FAILED;

	TShader* shader = createShaderObject<TShader>( pD3dDevice,
		pShaderBlob,
		nullptr );
	return shader;
}


}// namespace util