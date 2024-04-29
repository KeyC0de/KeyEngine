#include "mouse_picker.h"
#include "graphics.h"
#include "camera.h"
#include "console.h"
#include "camera_manager.h"


namespace dx = DirectX;

#pragma warning( disable : 4244 )
MousePicker::MousePicker( Graphics &gph,
	int screenX,
	int screenY )
{
	// convert to NDC space
	dx::XMFLOAT2 coordsNdc = convertToNdc( gph, dx::XMFLOAT2(screenX, screenY) );

	// convert to clip space, account for the aspect ratio of the viewport
	dx::XMFLOAT2 coordsClip{convertToClip( gph, coordsNdc )};

	// convert to view space
	dx::XMFLOAT4 coordsView{};
	dx::XMStoreFloat4( &coordsView, convertToViewSpace( gph, coordsClip ) );

	// convert to world space
	dx::XMStoreFloat3( &m_rayDirectionWorldSpace, convertToWorldSpace( gph, coordsView ) );

	// normalize the ray direction
	auto vec = dx::XMVector3Normalize( dx::XMLoadFloat3( &m_rayDirectionWorldSpace ) );
	dx::XMStoreFloat3( &m_rayDirectionWorldSpace, vec );

#if defined _DEBUG && !defined NDEBUG
	KeyConsole &console = KeyConsole::getInstance();
	console.print( "Ray WS direction = (" + std::to_string( m_rayDirectionWorldSpace.x )
		+ ",y=" + std::to_string( m_rayDirectionWorldSpace.y )
		+ ",z=" + std::to_string( m_rayDirectionWorldSpace.z ) + ")" );
#endif

	// the origin of the picking ray is the position of the camera
	auto &camMan = CameraManager::getInstance();
	const auto camPos{camMan.getActiveCamera().getPosition()};

	// now perform ray intersection test...
}
#pragma warning( default : 4244 )

DirectX::XMFLOAT2 MousePicker::convertToNdc( Graphics &gph,
	const dx::XMFLOAT2 &coordsScreenSpace )
{
	return {(coordsScreenSpace.x * 2.0f) / gph.getClientWidth() - 1.0f, 1.0f - (coordsScreenSpace.y * 2.0f) / gph.getClientHeight()};
}

DirectX::XMFLOAT2 MousePicker::convertToClip( Graphics &gph,
	const dx::XMFLOAT2 &coordsNdc )
{
	dx::XMMATRIX projectionMatrix{gph.getProjectionMatrix()};
	dx::XMVECTOR row1 = projectionMatrix.r[0];
	dx::XMFLOAT4 row1f{};
	dx::XMStoreFloat4( &row1f, row1 );
	float zoomX = row1f.x;

	dx::XMVECTOR row2 = projectionMatrix.r[1];
	dx::XMFLOAT4 row2f{};
	dx::XMStoreFloat4( &row2f, row2 );
	float zoomY = row2f.y;
	return {coordsNdc.x / zoomX, coordsNdc.y / zoomY};
}

DirectX::XMVECTOR MousePicker::convertToViewSpace( Graphics &gph,
	const DirectX::XMFLOAT2 &coordsClip )
{
	dx::XMMATRIX invProjMatrix{dx::XMMatrixInverse( nullptr, gph.getProjectionMatrix() )};
	return dx::XMVector3Transform( dx::XMLoadFloat2( &coordsClip ), invProjMatrix );
}

DirectX::XMVECTOR MousePicker::convertToWorldSpace( Graphics &gph,
	const DirectX::XMFLOAT4 &coordsView )
{
	dx::XMMATRIX invViewMatrix{dx::XMMatrixInverse( nullptr, gph.getViewMatrix() )};
	return dx::XMVector4Transform( dx::XMLoadFloat4( &coordsView ), invViewMatrix );
}