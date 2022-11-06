#pragma once

#include <string>
#include <DirectXMath.h>
#include "camera_widget.h"
#include "frustum.h"
#include "rendering_channel.h"


class Graphics;

namespace ren
{
	class Renderer;
}

class Camera
{
	float m_travelSpeed = 12.0f;
	float m_rotationSpeed = 0.004f;
	float m_nearZ;
	float m_farZ;
	// tethered flag indicates whether the Camera's position is bound to another object
	// (eg shadow casting light), st when the object moves the camera moves along with it
	bool m_bTethered;
	std::string m_name;
	float m_aspectRatio;
	float m_fovRadians;
	DirectX::XMFLOAT3 m_position;
	float m_pitch;
	float m_yaw;
	DirectX::XMFLOAT3 m_homePosition;
	float m_homePitch;
	float m_homeYaw;
	bool m_bShowWidget = false;
	bool m_bShowFrustum = false;
	float m_width;
	float m_height;
	float m_homeWidth;
	float m_homeHeight;
	float m_homeNearZ;
	float m_homeFarZ;
	CameraWidget m_widget;
	Frustum m_cameraFrustum;
public:
	static DirectX::XMMATRIX getShadowOrthographicMatrix( const unsigned w, const unsigned h ) noexcept;
	static DirectX::XMMATRIX getShadowProjectionMatrix( const float farZ = 9999 ) noexcept;
public:
	Camera( Graphics &gph, const std::string &name, const int width, const int height, const float fovDegrees = 90.0f, const DirectX::XMFLOAT3 &homePos = {0.0f, 0.0f, 0.0f}, const float homePitch = 0.0f, const float homeYaw = 0.0f, const bool bTethered = false, const float nearZ = 0.5f, const float farZ = 400.0f ) noexcept;

	void render( const size_t channel = rch::all ) const;
	void makeActive( Graphics &gph, bool bOrthographic ) const;
	DirectX::XMMATRIX getViewMatrix() const noexcept;
	DirectX::XMMATRIX getReflectionViewMatrix( const DirectX::XMVECTOR &mirrorPlane ) const noexcept;
	DirectX::XMMATRIX getPerspectiveProjectionMatrix() const noexcept;
	DirectX::XMMATRIX getOrthographicProjectionMatrix( const unsigned viewWidth, const unsigned viewHeight ) const noexcept;
	void displayImguiWidgets( Graphics &gph ) noexcept;
	const float getFovRadians() const noexcept;
	void resetToDefault( Graphics &gph ) noexcept;
	// rotate arguments is mouse dx, dy delta values
	void rotateRel( const float dx, const float dy ) noexcept;
	void translateRel( DirectX::XMFLOAT3 translation ) noexcept;
	const DirectX::XMFLOAT3& getPosition() const noexcept;
	//	\function	calcDirection	||	\date	2022/08/30 23:11
	//	\brief	camDirection = camPosition - camTarget
	DirectX::XMVECTOR calcDirection() const noexcept;
	DirectX::XMVECTOR calcRight() const noexcept;
	DirectX::XMVECTOR calcUp() const noexcept;
	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	const std::string& getName() const noexcept;
	void connectEffectsToRenderer( ren::Renderer &ren );
	std::vector<DirectX::XMFLOAT4> getFrustumPlanes() const noexcept;
};

using Projector = Camera;