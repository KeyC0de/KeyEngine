#include "d3d_utils.h"
#include "math_utils.h"
#include "DirectXTK/SpriteFont.h"
#include "DirectXTK/SpriteBatch.h"


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

std::optional<DirectX::XMVECTORF32> getDirectXColorFromString( const std::string &dxColorString )
{
	if ( dxColorString == "AliceBlue" )
	{
		return DirectX::Colors::AliceBlue;
	}
	if ( dxColorString == "AntiqueWhite" )
	{
		return DirectX::Colors::AntiqueWhite;
	}
	if ( dxColorString == "Aqua" )
	{
		return DirectX::Colors::Aqua;
	}
	if ( dxColorString == "Aquamarine" )
	{
		return DirectX::Colors::Aquamarine;
	}
	if ( dxColorString == "Azure" )
	{
		return DirectX::Colors::Azure;
	}
	if ( dxColorString == "Beige" )
	{
		return DirectX::Colors::Beige;
	}
	if ( dxColorString == "Bisque" )
	{
		return DirectX::Colors::Bisque;
	}
	if ( dxColorString == "Black" )
	{
		return DirectX::Colors::Black;
	}
	if ( dxColorString == "BlanchedAlmond" )
	{
		return DirectX::Colors::BlanchedAlmond;
	}
	if ( dxColorString == "Blue" )
	{
		return DirectX::Colors::Blue;
	}
	if ( dxColorString == "BlueViolet" )
	{
		return DirectX::Colors::BlueViolet;
	}
	if ( dxColorString == "Brown" )
	{
		return DirectX::Colors::Brown;
	}
	if ( dxColorString == "BurlyWood" )
	{
		return DirectX::Colors::BurlyWood;
	}
	if ( dxColorString == "CadetBlue" )
	{
		return DirectX::Colors::CadetBlue;
	}
	if ( dxColorString == "Chartreuse" )
	{
		return DirectX::Colors::Chartreuse;
	}
	if ( dxColorString == "Chocolate" )
	{
		return DirectX::Colors::Chocolate;
	}
	if ( dxColorString == "Coral" )
	{
		return DirectX::Colors::Coral;
	}
	if ( dxColorString == "CornflowerBlue" )
	{
		return DirectX::Colors::CornflowerBlue;
	}
	if ( dxColorString == "Cornsilk" )
	{
		return DirectX::Colors::Cornsilk;
	}
	if ( dxColorString == "Crimson" )
	{
		return DirectX::Colors::Crimson;
	}
	if ( dxColorString == "Cyan" )
	{
		return DirectX::Colors::Cyan;
	}
	if ( dxColorString == "DarkBlue" )
	{
		return DirectX::Colors::DarkBlue;
	}
	if ( dxColorString == "DarkCyan" )
	{
		return DirectX::Colors::DarkCyan;
	}
	if ( dxColorString == "DarkGoldenrod" )
	{
		return DirectX::Colors::DarkGoldenrod;
	}
	if ( dxColorString == "DarkGray" )
	{
		return DirectX::Colors::DarkGray;
	}
	if ( dxColorString == "DarkGreen" )
	{
		return DirectX::Colors::DarkGreen;
	}
	if ( dxColorString == "DarkKhaki" )
	{
		return DirectX::Colors::DarkKhaki;
	}
	if ( dxColorString == "DarkMagenta" )
	{
		return DirectX::Colors::DarkMagenta;
	}
	if ( dxColorString == "DarkOliveGreen" )
	{
		return DirectX::Colors::DarkOliveGreen;
	}
	if ( dxColorString == "DarkOrange" )
	{
		return DirectX::Colors::DarkOrange;
	}
	if ( dxColorString == "DarkOrchid" )
	{
		return DirectX::Colors::DarkOrchid;
	}
	if ( dxColorString == "DarkRed" )
	{
		return DirectX::Colors::DarkRed;
	}
	if ( dxColorString == "DarkSalmon" )
	{
		return DirectX::Colors::DarkSalmon;
	}
	if ( dxColorString == "DarkSeaGreen" )
	{
		return DirectX::Colors::DarkSeaGreen;
	}
	if ( dxColorString == "DarkSlateBlue" )
	{
		return DirectX::Colors::DarkSlateBlue;
	}
	if ( dxColorString == "DarkSlateGray" )
	{
		return DirectX::Colors::DarkSlateGray;
	}
	if ( dxColorString == "DarkTurquoise" )
	{
		return DirectX::Colors::DarkTurquoise;
	}
	if ( dxColorString == "DarkViolet" )
	{
		return DirectX::Colors::DarkViolet;
	}
	if ( dxColorString == "DeepPink" )
	{
		return DirectX::Colors::DeepPink;
	}
	if ( dxColorString == "DeepSkyBlue" )
	{
		return DirectX::Colors::DeepSkyBlue;
	}
	if ( dxColorString == "DimGray" )
	{
		return DirectX::Colors::DimGray;
	}
	if ( dxColorString == "DodgerBlue" )
	{
		return DirectX::Colors::DodgerBlue;
	}
	if ( dxColorString == "Firebrick" )
	{
		return DirectX::Colors::Firebrick;
	}
	if ( dxColorString == "FloralWhite" )
	{
		return DirectX::Colors::FloralWhite;
	}
	if ( dxColorString == "ForestGreen" )
	{
		return DirectX::Colors::ForestGreen;
	}
	if ( dxColorString == "Fuchsia" )
	{
		return DirectX::Colors::Fuchsia;
	}
	if ( dxColorString == "Gainsboro" )
	{
		return DirectX::Colors::Gainsboro;
	}
	if ( dxColorString == "GhostWhite" )
	{
		return DirectX::Colors::GhostWhite;
	}
	if ( dxColorString == "Gold" )
	{
		return DirectX::Colors::Gold;
	}
	if ( dxColorString == "Goldenrod" )
	{
		return DirectX::Colors::Goldenrod;
	}
	if ( dxColorString == "Gray" )
	{
		return DirectX::Colors::Gray;
	}
	if ( dxColorString == "Green" )
	{
		return DirectX::Colors::Green;
	}
	if ( dxColorString == "GreenYellow" )
	{
		return DirectX::Colors::GreenYellow;
	}
	if ( dxColorString == "Honeydew" )
	{
		return DirectX::Colors::Honeydew;
	}
	if ( dxColorString == "HotPink" )
	{
		return DirectX::Colors::HotPink;
	}
	if ( dxColorString == "IndianRed" )
	{
		return DirectX::Colors::IndianRed;
	}
	if ( dxColorString == "Indigo" )
	{
		return DirectX::Colors::Indigo;
	}
	if ( dxColorString == "Ivory" )
	{
		return DirectX::Colors::Ivory;
	}
	if ( dxColorString == "Khaki" )
	{
		return DirectX::Colors::Khaki;
	}
	if ( dxColorString == "Lavender" )
	{
		return DirectX::Colors::Lavender;
	}
	if ( dxColorString == "LavenderBlush" )
	{
		return DirectX::Colors::LavenderBlush;
	}
	if ( dxColorString == "LawnGreen" )
	{
		return DirectX::Colors::LawnGreen;
	}
	if ( dxColorString == "LemonChiffon" )
	{
		return DirectX::Colors::LemonChiffon;
	}
	if ( dxColorString == "LightBlue" )
	{
		return DirectX::Colors::LightBlue;
	}
	if ( dxColorString == "LightCoral" )
	{
		return DirectX::Colors::LightCoral;
	}
	if ( dxColorString == "LightCyan" )
	{
		return DirectX::Colors::LightCyan;
	}
	if ( dxColorString == "LightGoldenrodYellow" )
	{
		return DirectX::Colors::LightGoldenrodYellow;
	}
	if ( dxColorString == "LightGreen" )
	{
		return DirectX::Colors::LightGreen;
	}
	if ( dxColorString == "LightGray" )
	{
		return DirectX::Colors::LightGray;
	}
	if ( dxColorString == "LightPink" )
	{
		return DirectX::Colors::LightPink;
	}
	if ( dxColorString == "LightSalmon" )
	{
		return DirectX::Colors::LightSalmon;
	}
	if ( dxColorString == "LightSeaGreen" )
	{
		return DirectX::Colors::LightSeaGreen;
	}
	if ( dxColorString == "LightSkyBlue" )
	{
		return DirectX::Colors::LightSkyBlue;
	}
	if ( dxColorString == "LightSlateGray" )
	{
		return DirectX::Colors::LightSlateGray;
	}
	if ( dxColorString == "LightSteelBlue" )
	{
		return DirectX::Colors::LightSteelBlue;
	}
	if ( dxColorString == "LightYellow" )
	{
		return DirectX::Colors::LightYellow;
	}
	if ( dxColorString == "Lime" )
	{
		return DirectX::Colors::Lime;
	}
	if ( dxColorString == "LimeGreen" )
	{
		return DirectX::Colors::LimeGreen;
	}
	if ( dxColorString == "Linen" )
	{
		return DirectX::Colors::Linen;
	}
	if ( dxColorString == "Magenta" )
	{
		return DirectX::Colors::Magenta;
	}
	if ( dxColorString == "Maroon" )
	{
		return DirectX::Colors::Maroon;
	}
	if ( dxColorString == "MediumAquamarine" )
	{
		return DirectX::Colors::MediumAquamarine;
	}
	if ( dxColorString == "MediumBlue" )
	{
		return DirectX::Colors::MediumBlue;
	}
	if ( dxColorString == "MediumOrchid" )
	{
		return DirectX::Colors::MediumOrchid;
	}
	if ( dxColorString == "MediumPurple" )
	{
		return DirectX::Colors::MediumPurple;
	}
	if ( dxColorString == "MediumSeaGreen" )
	{
		return DirectX::Colors::MediumSeaGreen;
	}
	if ( dxColorString == "MediumSlateBlue" )
	{
		return DirectX::Colors::MediumSlateBlue;
	}
	if ( dxColorString == "MediumSpringGreen" )
	{
		return DirectX::Colors::MediumSpringGreen;
	}
	if ( dxColorString == "MediumTurquoise" )
	{
		return DirectX::Colors::MediumTurquoise;
	}
	if ( dxColorString == "MediumVioletRed" )
	{
		return DirectX::Colors::MediumVioletRed;
	}
	if ( dxColorString == "MidnightBlue" )
	{
		return DirectX::Colors::MidnightBlue;
	}
	if ( dxColorString == "MintCream" )
	{
		return DirectX::Colors::MintCream;
	}
	if ( dxColorString == "MistyRose" )
	{
		return DirectX::Colors::MistyRose;
	}
	if ( dxColorString == "Moccasin" )
	{
		return DirectX::Colors::Moccasin;
	}
	if ( dxColorString == "NavajoWhite" )
	{
		return DirectX::Colors::NavajoWhite;
	}
	if ( dxColorString == "Navy" )
	{
		return DirectX::Colors::Navy;
	}
	if ( dxColorString == "OldLace" )
	{
		return DirectX::Colors::OldLace;
	}
	if ( dxColorString == "Olive" )
	{
		return DirectX::Colors::Olive;
	}
	if ( dxColorString == "OliveDrab" )
	{
		return DirectX::Colors::OliveDrab;
	}
	if ( dxColorString == "Orange" )
	{
		return DirectX::Colors::Orange;
	}
	if ( dxColorString == "OrangeRed" )
	{
		return DirectX::Colors::OrangeRed;
	}
	if ( dxColorString == "Orchid" )
	{
		return DirectX::Colors::Orchid;
	}
	if ( dxColorString == "PaleGoldenrod" )
	{
		return DirectX::Colors::PaleGoldenrod;
	}
	if ( dxColorString == "PaleGreen" )
	{
		return DirectX::Colors::PaleGreen;
	}
	if ( dxColorString == "PaleTurquoise" )
	{
		return DirectX::Colors::PaleTurquoise;
	}
	if ( dxColorString == "PaleVioletRed" )
	{
		return DirectX::Colors::PaleVioletRed;
	}
	if ( dxColorString == "PapayaWhip" )
	{
		return DirectX::Colors::PapayaWhip;
	}
	if ( dxColorString == "PeachPuff" )
	{
		return DirectX::Colors::PeachPuff;
	}
	if ( dxColorString == "Peru" )
	{
		return DirectX::Colors::Peru;
	}
	if ( dxColorString == "Pink" )
	{
		return DirectX::Colors::Pink;
	}
	if ( dxColorString == "Plum" )
	{
		return DirectX::Colors::Plum;
	}
	if ( dxColorString == "PowderBlue" )
	{
		return DirectX::Colors::PowderBlue;
	}
	if ( dxColorString == "Purple" )
	{
		return DirectX::Colors::Purple;
	}
	if ( dxColorString == "Red" )
	{
		return DirectX::Colors::Red;
	}
	if ( dxColorString == "RosyBrown" )
	{
		return DirectX::Colors::RosyBrown;
	}
	if ( dxColorString == "RoyalBlue" )
	{
		return DirectX::Colors::RoyalBlue;
	}
	if ( dxColorString == "SaddleBrown" )
	{
		return DirectX::Colors::SaddleBrown;
	}
	if ( dxColorString == "Salmon" )
	{
		return DirectX::Colors::Salmon;
	}
	if ( dxColorString == "SandyBrown" )
	{
		return DirectX::Colors::SandyBrown;
	}
	if ( dxColorString == "SeaGreen" )
	{
		return DirectX::Colors::SeaGreen;
	}
	if ( dxColorString == "SeaShell" )
	{
		return DirectX::Colors::SeaShell;
	}
	if ( dxColorString == "Sienna" )
	{
		return DirectX::Colors::Sienna;
	}
	if ( dxColorString == "Silver" )
	{
		return DirectX::Colors::Silver;
	}
	if ( dxColorString == "SkyBlue" )
	{
		return DirectX::Colors::SkyBlue;
	}
	if ( dxColorString == "SlateBlue" )
	{
		return DirectX::Colors::SlateBlue;
	}
	if ( dxColorString == "SlateGray" )
	{
		return DirectX::Colors::SlateGray;
	}
	if ( dxColorString == "Snow" )
	{
		return DirectX::Colors::Snow;
	}
	if ( dxColorString == "SpringGreen" )
	{
		return DirectX::Colors::SpringGreen;
	}
	if ( dxColorString == "SteelBlue" )
	{
		return DirectX::Colors::SteelBlue;
	}
	if ( dxColorString == "Tan" )
	{
		return DirectX::Colors::Tan;
	}
	if ( dxColorString == "Teal" )
	{
		return DirectX::Colors::Teal;
	}
	if ( dxColorString == "Thistle" )
	{
		return DirectX::Colors::Thistle;
	}
	if ( dxColorString == "Tomato" )
	{
		return DirectX::Colors::Tomato;
	}
	if ( dxColorString == "Transparent" )
	{
		return DirectX::Colors::Transparent;
	}
	if ( dxColorString == "Turquoise" )
	{
		return DirectX::Colors::Turquoise;
	}
	if ( dxColorString == "Violet" )
	{
		return DirectX::Colors::Violet;
	}
	if ( dxColorString == "Wheat" )
	{
		return DirectX::Colors::Wheat;
	}
	if ( dxColorString == "White" )
	{
		return DirectX::Colors::White;
	}
	if ( dxColorString == "WhiteSmoke" )
	{
		return DirectX::Colors::WhiteSmoke;
	}
	if ( dxColorString == "Yellow" )
	{
		return DirectX::Colors::Yellow;
	}
	if ( dxColorString == "YellowGreen" )
	{
		return DirectX::Colors::YellowGreen;
	}

	return std::nullopt;
}


}//util