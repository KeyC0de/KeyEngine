#pragma once

#include <string>
#include <DirectXMath.h>
#include "camera_widget.h"
#include "frustum.h"


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
	static DirectX::XMMATRIX getShadowOrthographicMatrix( unsigned w, unsigned h ) noexcept;
	static DirectX::XMMATRIX getShadowProjectionMatrix( float farZ = 9999 ) noexcept;
public:
	Camera( Graphics &gph, const std::string &name, int width = 1280, int height = 720,
		float fovDegrees = 90.0f,
		const DirectX::XMFLOAT3 &homePos = {0.0f, 0.0f, 0.0f},
		float homePitch = 0.0f, float homeYaw = 0.0f, bool bTethered = false,
		float nearZ = 0.5f, float farZ = 400.0f ) noexcept;

	void render( size_t channel ) const;
	void makeActive( Graphics &gph, bool bOrthographic ) const;
	DirectX::XMMATRIX getViewMatrix() const noexcept;
	DirectX::XMMATRIX getReflectionViewMatrix( const DirectX::XMVECTOR &mirrorPlane ) const
		noexcept;
	DirectX::XMMATRIX getPerspectiveProjectionMatrix() const noexcept;
	DirectX::XMMATRIX getOrthographicProjectionMatrix( unsigned viewWidth,
		unsigned viewHeight ) const noexcept;
	void displayImguiWidgets( Graphics &gph ) noexcept;
	float getFovRadians() const noexcept;
	void resetToDefault( Graphics &gph ) noexcept;
	// rotate arguments is mouse dx, dy delta values
	void rotateRel( float dx, float dy ) noexcept;
	void translateRel( DirectX::XMFLOAT3 translation ) noexcept;
	DirectX::XMFLOAT3 getPosition() const noexcept;
	DirectX::XMVECTOR getDirection() const noexcept;
	DirectX::XMVECTOR getRight() const noexcept;
	DirectX::XMVECTOR getUp() const noexcept;
	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	const std::string& getName() const noexcept;
	void connectEffectsToRenderer( ren::Renderer &ren );
};

using Projector = Camera;