#pragma once

#include "winner.h"
#include <string>
#include <vector>
#include <optional>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "assertions_console.h"
#include "os_utils.h"
#include "key_wrl.h"
#include "utils.h"


namespace util
{

//	\function	comSafeRelease	||	\date	2018/12/30 17:35
//	\brief	safely release a COM object	---	deprecated
template<typename T>
inline void comSafeRelease( T &p )
{
	if ( p != nullptr )
	{
		p->Release();
		p = nullptr;
	}
}

static auto s_XMZero = DirectX::XMVectorZero();

DirectX::XMMATRIX XM_CALLCONV computeTransform( const DirectX::XMFLOAT3 &scale, const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT3 &rot );

//	\function	extractRotation	||	\date	2022/09/03 14:37
//	\brief	builds a FLOAT3 of {x=pitch,y=yaw,z=roll}
DirectX::XMFLOAT3 extractRotation( const DirectX::XMFLOAT4X4 &mat );
ALIAS_FUNCTION( extractRotation, extractEulerAngles );
float getPitch( const DirectX::XMFLOAT4X4 &mat );
float getYaw( const DirectX::XMFLOAT4X4 &mat );
float getRoll( const DirectX::XMFLOAT4X4 &mat );
DirectX::XMFLOAT3 extractTranslation( const DirectX::XMFLOAT4X4 &mat );
DirectX::XMMATRIX XM_CALLCONV scaleTranslation( const DirectX::XMMATRIX &mat, const float scale );
DirectX::XMVECTOR XM_CALLCONV pitchYawRollToQuaternion( const DirectX::XMVECTOR& pitchYawRollAngles );
ALIAS_FUNCTION( pitchYawRollToQuaternion, eulerAnglesToQuaternion );
void quaternionToEulerAngles( DirectX::XMFLOAT4 &quat, float &pitch, float &yaw, float &roll );

//	\function	quaternionToPitchYawRoll	||	\date	2022/09/03 14:48
//	\brief	same as quaternionToEulerAngles except this version returns the euler angles in a float3 vector
DirectX::XMFLOAT3 quaternionToPitchYawRoll( DirectX::XMFLOAT4 &quat );
DirectX::XMVECTOR XM_CALLCONV pitchYawRollToVector( const float pitch, const float yaw, const float roll );
float distance( const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2 ) noexcept;
ALIAS_FUNCTION( distance, length );
float distanceSquared( const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2 ) noexcept;
ALIAS_FUNCTION( distanceSquared, lengthSquared );
DirectX::XMVECTOR XM_CALLCONV addVectorWithScalar( const DirectX::XMVECTOR &v, const float scalar ) noexcept;
DirectX::XMVECTOR XM_CALLCONV subtractScalarFromVector( const DirectX::XMVECTOR &v, const float scalar ) noexcept;
DirectX::XMVECTOR XM_CALLCONV multiplyVectorWithScalar( const DirectX::XMVECTOR &v, const float scalar ) noexcept;
DirectX::XMVECTOR XM_CALLCONV divideVectorWithScalar( const DirectX::XMVECTOR &v, const float scalar ) noexcept;
DirectX::XMFLOAT3 getMidpointOfTriangle( const DirectX::XMFLOAT3 &v0, const DirectX::XMFLOAT3 &v1, const DirectX::XMFLOAT3 &v2 ) noexcept;

//	\function	rotateVectorAroundAxisAngle	||	\date	2024/04/18 13:39
//	\brief	rotate a vector around an axis and an angle (in degrees) using Rodrigues' rotation formula:
//			axis * cos( theta ) + ( axis.Dot( v ) * v * ( 1 -cos( theta ) ) + ( v.Cross( axis ) * sin( theta ) );
DirectX::XMVECTOR rotateVectorAroundAxisAngle( const DirectX::XMVECTOR &v, const DirectX::XMVECTOR &axis, const float theta ) noexcept;

DXGI_RATIONAL queryRefreshRate( const unsigned screenWidth, const unsigned screenHeight, const bool bVsync );

template<class TShader>
static TShader* createShaderObject( ID3D11Device *pD3dDevice, ID3DBlob *&pShaderBlob, ID3D11ClassLinkage *pClassLinkage );

template<>
static ID3D11VertexShader* createShaderObject<ID3D11VertexShader>( ID3D11Device *pD3dDevice,
	ID3DBlob *&pShaderBlob,
	ID3D11ClassLinkage *pClassLinkage )
{
	ASSERT( pD3dDevice, "d3d11 Device is null!" );
	ASSERT( pShaderBlob, "d3d11 shader object is null!" );

	ID3D11VertexShader *pVertexShader;
	HRESULT hres = pD3dDevice->CreateVertexShader( pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pVertexShader );
	ASSERT_HRES_IF_FAILED;

	return pVertexShader;
}

template<>
static ID3D11PixelShader* createShaderObject<ID3D11PixelShader>( ID3D11Device *pD3dDevice,
	ID3DBlob *&pShaderBlob,
	ID3D11ClassLinkage *pClassLinkage )
{
	ASSERT( pD3dDevice, "d3d11 Device is null!" );
	ASSERT( pShaderBlob, "d3d11 shader object is null!" );

	ID3D11PixelShader *pPixelShader;
	HRESULT hres = pD3dDevice->CreatePixelShader( pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pPixelShader );
	ASSERT_HRES_IF_FAILED;

	return pPixelShader;
}

//	\function	loadShaderDynamically	||	\date	2019/12/30 17:28
//	\brief	will be used to load and compile a shader at runtime
template<class TShader>
TShader* loadShaderDynamically( ID3D11Device *pD3dDevice,
	const std::wstring &fileName,
	const std::string &entryPoint,
	const std::string &profile,
	ID3DBlob *&pShaderBlob,
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

	HRESULT hres = D3DCompileFromFile( fileName.data(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), queryFeatureLevel().c_str(), flags, 0u, &pShaderBlob, &pErrorBlob );
	if ( pErrorBlob )
	{
		OutputDebugStringW( (LPCWSTR) pErrorBlob->GetBufferPointer() );
	}
	ASSERT_HRES_IF_FAILED;

	TShader *shader = createShaderObject<TShader>( pD3dDevice, pShaderBlob, nullptr );
	return shader;
}

ALIAS_FUNCTION( loadShaderDynamically, compileShaderDynamically );

std::optional<DirectX::XMVECTORF32> getDirectXColorFromString( const std::string &dxColorString );

}// namespace util