#pragma once

#include "winner.h"
#include <string>
#include <vector>
#include <tuple>
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

/// \brief	safely release a COM object	---	deprecated
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

bool isNormalized( const DirectX::XMFLOAT3 &vec );
DirectX::XMMATRIX XM_CALLCONV computeTransform( const DirectX::XMFLOAT3 &scale, const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT3 &rot );
/// \brief	builds a FLOAT3 of {x=pitch,y=yaw,z=roll}
DirectX::XMFLOAT3 extractRotation( const DirectX::XMFLOAT4X4 &mat );
ALIAS_FUNCTION( extractRotation, extractEulerAngles );
float getPitch( const DirectX::XMFLOAT4X4 &mat );
float getYaw( const DirectX::XMFLOAT4X4 &mat );
float getRoll( const DirectX::XMFLOAT4X4 &mat );
bool operator==( const DirectX::XMFLOAT3 &lhs, const DirectX::XMFLOAT3 &rhs ) noexcept;
bool operator!=( const DirectX::XMFLOAT3 &lhs, const DirectX::XMFLOAT3 &rhs ) noexcept;
bool operator==( const DirectX::XMFLOAT4 &lhs, const DirectX::XMFLOAT4 &rhs ) noexcept;
bool operator!=( const DirectX::XMFLOAT4 &lhs, const DirectX::XMFLOAT4 &rhs ) noexcept;
bool operator==( const DirectX::XMVECTOR &lhs, const DirectX::XMVECTOR &rhs ) noexcept;
bool operator!=( const DirectX::XMVECTOR &lhs, const DirectX::XMVECTOR &rhs ) noexcept;
bool operator==( const DirectX::XMMATRIX &lhs, const DirectX::XMMATRIX &rhs ) noexcept;
bool operator!=( const DirectX::XMMATRIX &lhs, const DirectX::XMMATRIX &rhs ) noexcept;
DirectX::XMFLOAT3 extractTranslation( const DirectX::XMFLOAT4X4 &mat );
DirectX::XMFLOAT3 extractTranslationColumnMatrix( const DirectX::XMFLOAT4X4 &mat );
DirectX::XMMATRIX XM_CALLCONV scaleTranslation( const DirectX::XMMATRIX &mat, const float scale );
DirectX::XMVECTOR XM_CALLCONV pitchYawRollToQuaternion( const DirectX::XMVECTOR& pitchYawRollAnglesVec );
DirectX::XMVECTOR XM_CALLCONV pitchYawRollToQuaternion( const DirectX::XMFLOAT3& pitchYawRollAngles );
ALIAS_FUNCTION( pitchYawRollToQuaternion, eulerAnglesToQuaternion );
void quaternionToEulerAngles( const DirectX::XMFLOAT4 &quat, float &pitch, float &yaw, float &roll );
DirectX::XMFLOAT3 quaternionToEulerAngles( const DirectX::XMFLOAT4 &quat );
std::tuple<DirectX::XMFLOAT3, DirectX::XMFLOAT3, DirectX::XMFLOAT3> decomposeAffineMatrix( const DirectX::XMFLOAT4X4 &transform );
std::tuple<DirectX::XMFLOAT3, DirectX::XMFLOAT3, DirectX::XMFLOAT3> decomposeAffineMatrix( const DirectX::XMMATRIX &transform );

void toDegrees3InPlace( DirectX::XMFLOAT3 &rotAngles );
DirectX::XMFLOAT3 toDegrees3( const DirectX::XMFLOAT3 &rotAngles );
void toRadians3InPlace( DirectX::XMFLOAT3 &rotAngles );
DirectX::XMFLOAT3 toRadians3( const DirectX::XMFLOAT3 &rotAngles );

/// \brief	same as quaternionToEulerAngles except this version returns the euler angles in a float3 vector
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

/// \brief	rotate a vector around an axis and an angle (in degrees) using Rodrigues' rotation formula:
/// \brief	axis * cos( theta ) + ( axis.Dot( v ) * v * ( 1 -cos( theta ) ) + ( v.Cross( axis ) * sin( theta ) );
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

/// \brief	will be used to load and compile a shader at runtime
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