#include "d3d_utils.h"


namespace util
{

namespace dx = DirectX;


dx::XMFLOAT3 extractEulerAngles( const dx::XMFLOAT4X4 &mat )
{
	dx::XMFLOAT3 eulerRot;
	
	eulerRot.x = asinf( -mat._32 );					// Pitch
	if ( cosf( eulerRot.x ) > 0.0001 )				// Not at poles
	{
		eulerRot.y = atan2f( mat._31, mat._33 );	// Yaw
		eulerRot.z = atan2f( mat._12, mat._22 );	// Roll
	}
	else
	{
		eulerRot.y = 0.0f;							// Yaw
		eulerRot.z = atan2f( -mat._21, mat._11 );	// Roll
	}

	return eulerRot;
}

dx::XMFLOAT3 extractTranslation( const dx::XMFLOAT4X4 &mat )
{
	return {mat._41, mat._42, mat._43};
}

dx::XMMATRIX scaleTranslation( const dx::XMMATRIX &mat,
	float scale )
{
	dx::XMMATRIX tmp{mat};
	tmp.r[3].m128_f32[0] *= scale;
	tmp.r[3].m128_f32[1] *= scale;
	tmp.r[3].m128_f32[2] *= scale;
	return tmp;
}

DXGI_RATIONAL queryRefreshRate( unsigned screenWidth,
	unsigned screenHeight,
	bool bVsync )
{
	DXGI_RATIONAL refreshRate = {0, 1};
	if ( bVsync )
	{
		IDXGIFactory *factory;
		IDXGIAdapter *adapter;
		IDXGIOutput *adapterOutput;
		DXGI_MODE_DESC *displayModeList;

		// Create a DirectX graphics interface factory.
		HRESULT hres = CreateDXGIFactory1( __uuidof( IDXGIFactory1 ),
			(void**)&factory );
		ASSERT_HRES_IF_FAILED;

		hres = factory->EnumAdapters( 0,
			&adapter );
		ASSERT_HRES_IF_FAILED;

		hres = adapter->EnumOutputs( 0,
			&adapterOutput );
		ASSERT_HRES_IF_FAILED;

		UINT numDisplayModes;
		hres = adapterOutput->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM,
			DXGI_ENUM_MODES_INTERLACED,
			&numDisplayModes,
			nullptr );
		ASSERT_HRES_IF_FAILED;

		displayModeList = new DXGI_MODE_DESC[numDisplayModes];
		ASSERT( displayModeList, "display mode array is null!" );

		hres = adapterOutput->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM,
			DXGI_ENUM_MODES_INTERLACED,
			&numDisplayModes,
			displayModeList );
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
		comSafeRelease( adapterOutput );
		comSafeRelease( adapter );
		comSafeRelease( factory );
	}

	return refreshRate;
}


}//util