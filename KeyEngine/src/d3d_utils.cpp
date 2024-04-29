#include "d3d_utils.h"
#include "math_utils.h"


namespace util
{

namespace dx = DirectX;


DirectX::XMMATRIX XM_CALLCONV computeTransform( const DirectX::XMFLOAT3 &scale,
	const DirectX::XMFLOAT3 &pos,
	const DirectX::XMFLOAT3 &rot )
{
	return dx::XMMatrixScaling( scale.x, scale.y, scale.z ) * dx::XMMatrixRotationX( rot.x ) * dx::XMMatrixRotationY( rot.y ) * dx::XMMatrixRotationZ( rot.z ) * dx::XMMatrixTranslation( pos.x, pos.y, pos.z );
}

dx::XMFLOAT3 extractRotation( const dx::XMFLOAT4X4 &mat )
{
	dx::XMFLOAT3 eulerRot{};

	eulerRot.x = asinf( -mat._32 );					// Pitch
	if ( cosf( eulerRot.x ) > 0.0001 )				// Not at poles
	{
		eulerRot.y = atan2f( mat._31, mat._33 );	// Yaw
		eulerRot.z = atan2f( mat._12, mat._22 );	// Roll
	}
	else
	{
		eulerRot.y = 0.0f;								// Yaw
		eulerRot.z = atan2f( -mat._21, mat._11 );	// Roll
	}

	return eulerRot;
}

float getPitch( const dx::XMFLOAT4X4 &mat )
{
	return asinf( -mat._32 );
}

float getYaw( const dx::XMFLOAT4X4 &mat )
{
	dx::XMFLOAT3 eulerRot{};

	eulerRot.x = asinf( -mat._32 );					// Pitch
	if ( cosf( eulerRot.x ) > 0.0001 )				// Not at poles
	{
		eulerRot.y = atan2f( mat._31, mat._33 );	// Yaw
	}
	else
	{
		eulerRot.y = 0.0f;								// Yaw
	}

	return eulerRot.y;
}

float getRoll( const dx::XMFLOAT4X4 &mat )
{
	dx::XMFLOAT3 eulerRot{};

	eulerRot.x = asinf( -mat._32 );					// Pitch
	if ( cosf( eulerRot.x ) > 0.0001 )				// Not at poles
	{
		eulerRot.z = atan2f( mat._12, mat._22 );	// Roll
	}
	else
	{
		eulerRot.z = atan2f( -mat._21, mat._11 );	// Roll
	}

	return eulerRot.z;
}

dx::XMFLOAT3 extractTranslation( const dx::XMFLOAT4X4 &mat )
{
	return {mat._41/* x */, mat._42/* y */, mat._43/* z */};
}

dx::XMMATRIX XM_CALLCONV scaleTranslation( const dx::XMMATRIX &mat,
	const float scale )
{
	dx::XMMATRIX tmp{mat};
	tmp.r[3].m128_f32[0] *= scale;
	tmp.r[3].m128_f32[1] *= scale;
	tmp.r[3].m128_f32[2] *= scale;
	return tmp;
}

dx::XMVECTOR XM_CALLCONV pitchYawRollToQuaternion( const dx::XMVECTOR& pitchYawRollAngles )
{
	return dx::XMQuaternionRotationRollPitchYawFromVector( pitchYawRollAngles );
}

void quaternionToEulerAngles( dx::XMFLOAT4 &quat,
	float &pitch,
	float &yaw,
	float &roll )
{
	pitch = asin( 2 * ( quat.w * quat.x - quat.z * quat.y ) );
	yaw = atan2( 2 * ( quat.w * quat.y + quat.x * quat.z ), 1 - 2 * ( quat.y * quat.y + quat.x * quat.x ) );
	roll = atan2( 2 * ( quat.w * quat.z + quat.y * quat.x ), 1 - 2 * ( quat.x * quat.x + quat.z * quat.z ) );
}

DirectX::XMFLOAT3 quaternionToPitchYawRoll( dx::XMFLOAT4 &quat )
{
	DirectX::XMFLOAT3 pitchYawRoll{asin( 2 * ( quat.w * quat.x - quat.z * quat.y ) ),
		atan2( 2 * ( quat.w * quat.y + quat.x * quat.z ), 1 - 2 * ( quat.y * quat.y + quat.x * quat.x ) ),
		atan2( 2 * ( quat.w * quat.z + quat.y * quat.x ), 1 - 2 * ( quat.x * quat.x + quat.z * quat.z ) )};
	return pitchYawRoll;
}

dx::XMVECTOR XM_CALLCONV pitchYawRollToVector( const float pitch,
	const float yaw,
	const float roll )
{
	dx::XMFLOAT4 angles{pitch, yaw, roll, 0.0f};
	return dx::XMLoadFloat4( &angles );
}

float distance( const DirectX::XMFLOAT3 &v1,
	const DirectX::XMFLOAT3 &v2 ) noexcept
{
	dx::XMVECTOR pos1 = dx::XMLoadFloat3( &v1 );
	dx::XMVECTOR pos2 = dx::XMLoadFloat3( &v2 );
	dx::XMVECTOR vectorSub = dx::XMVectorSubtract( pos1, pos2 );
	dx::XMVECTOR length = dx::XMVector3Length( vectorSub );

	float distance = 0.0f;
	dx::XMStoreFloat( &distance, length );
	return distance;
}

float distanceSquared( const DirectX::XMFLOAT3 &v1,
	const DirectX::XMFLOAT3 &v2 ) noexcept
{
	dx::XMVECTOR pos1 = dx::XMLoadFloat3( &v1 );
	dx::XMVECTOR pos2 = dx::XMLoadFloat3( &v2 );
	dx::XMVECTOR vectorSub = dx::XMVectorSubtract( pos1, pos2 );
	dx::XMVECTOR lengthSquared = dx::XMVector3LengthSq( vectorSub );

	float distanceSquared = 0.0f;
	dx::XMStoreFloat( &distanceSquared, lengthSquared );
	return distanceSquared;
}

DirectX::XMVECTOR XM_CALLCONV addVectorWithScalar( const DirectX::XMVECTOR &v,
	const float scalar ) noexcept
{
	DirectX::XMFLOAT4 vf{};
	XMStoreFloat4( &vf, v );
	vf.x += scalar;
	vf.y += scalar;
	vf.z += scalar;
	vf.w += scalar;
	return XMLoadFloat4( &vf );
}

DirectX::XMVECTOR XM_CALLCONV subtractScalarFromVector( const DirectX::XMVECTOR &v,
	const float scalar ) noexcept
{
	DirectX::XMFLOAT4 vf{};
	XMStoreFloat4( &vf, v );
	vf.x -= scalar;
	vf.y -= scalar;
	vf.z -= scalar;
	vf.w -= scalar;
	return XMLoadFloat4( &vf );
}

DirectX::XMVECTOR XM_CALLCONV multiplyVectorWithScalar( const DirectX::XMVECTOR &v,
	const float scalar ) noexcept
{
	using namespace DirectX;
	return v * scalar;
}

DirectX::XMVECTOR XM_CALLCONV divideVectorWithScalar( const DirectX::XMVECTOR &v,
	const float scalar ) noexcept
{
	using namespace DirectX;
	return v / scalar;
}

DirectX::XMFLOAT3 getMidpointOfTriangle( const DirectX::XMFLOAT3 &v0,
	const DirectX::XMFLOAT3 &v1,
	const DirectX::XMFLOAT3 &v2 ) noexcept
{
	return DirectX::XMFLOAT3{ (v0.x + v1.x + v2.x) / 3, (v0.y + v1.y + v2.y) / 3, (v0.z + v1.z + v2.z) / 3 };
}

DirectX::XMVECTOR rotateVectorAroundAxisAngle( const DirectX::XMVECTOR &v,
	const DirectX::XMVECTOR &axis,
	const float theta ) noexcept
{
	const float radians = util::toRadians( theta );
	dx::XMVECTOR quat = dx::XMQuaternionRotationAxis( axis, radians );
	return dx::XMVector3Rotate( v, quat );
}

DXGI_RATIONAL queryRefreshRate( const unsigned screenWidth,
	unsigned const screenHeight,
	const bool bVsync )
{
	DXGI_RATIONAL refreshRate = {0, 1};
	if ( bVsync )
	{
		IDXGIFactory *factory;
		IDXGIAdapter *adapter;
		IDXGIOutput *monitor;
		DXGI_MODE_DESC *displayModeList;

		// Create a DirectX graphics interface factory.
		HRESULT hres = CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), (void**)&factory );
		ASSERT_HRES_IF_FAILED;

		hres = factory->EnumAdapters( 0, &adapter );
		ASSERT_HRES_IF_FAILED;

		hres = adapter->EnumOutputs( 0, &monitor );
		ASSERT_HRES_IF_FAILED;

		UINT numDisplayModes;
		hres = monitor->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr );
		ASSERT_HRES_IF_FAILED;

		displayModeList = new DXGI_MODE_DESC[numDisplayModes];
		ASSERT( displayModeList, "display mode array is null!" );

		hres = monitor->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, displayModeList );
		ASSERT_HRES_IF_FAILED;

		// Now store the refresh rate of the monitor that matches the width and height
		//	of the requested screen.
		for ( std::size_t i = 0; i < numDisplayModes; ++i )
		{
			if ( displayModeList[i].Width == screenWidth &&
				displayModeList[i].Height == screenHeight )
			{
				refreshRate = displayModeList[i].RefreshRate;
			}
		}

		delete[] displayModeList;
		comSafeRelease( monitor );
		comSafeRelease( adapter );
		comSafeRelease( factory );
	}

	return refreshRate;
}


}//util