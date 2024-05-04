#pragma once

#include <string>
#include <DirectXMath.h>
#include "camera_widget.h"
#include "camera_frustum.h"
#include "rendering_channel.h"


class Graphics;

namespace ren
{
	class Renderer;
}

class Camera
{
	static constexpr inline float s_shadowFarZ = 100.0f;	// Shadows farZ hardcoded at 100units for now

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
	CameraWidget m_cameraWidget;
	CameraFrustum m_cameraFrustum;
public:
	static DirectX::XMMATRIX getShadowOrthographicMatrix( const unsigned w, const unsigned h ) noexcept;
	static DirectX::XMMATRIX getShadowProjectionMatrix( const float farZ = 9999 ) noexcept;
public:
	Camera( Graphics &gfx, const std::string &name, const int width, const int height, const float fovDegrees = 90.0f, const DirectX::XMFLOAT3 &homePos = {0.0f, 0.0f, 0.0f}, const float homePitch = 0.0f, const float homeYaw = 0.0f, const bool bTethered = false, const float nearZ = 0.5f, const float farZ = 200.0f ) noexcept;

	void render( const size_t channel = rch::all ) const;
	void connectEffectsToRenderer( ren::Renderer &ren );
	void makeActive( Graphics &gfx, const bool bOrthographic ) const;
	DirectX::XMMATRIX getViewMatrix() const noexcept;
	DirectX::XMMATRIX getReflectionViewMatrix( const DirectX::XMVECTOR &mirrorPlane ) const noexcept;
	DirectX::XMMATRIX getOrthographicProjectionMatrix( const unsigned width, const unsigned height ) const noexcept;
	DirectX::XMMATRIX getPerspectiveProjectionMatrix() const noexcept;
	void resetToDefault( Graphics &gfx ) noexcept;
	// rotate arguments is mouse dx, dy delta values
	void rotateRel( const float dx, const float dy ) noexcept;
	void translateRel( DirectX::XMFLOAT3 translation ) noexcept;
	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	//	\function	getTransform	||	\date	2024/04/28 13:10
	//	\brief	gets the camera matrix
	DirectX::XMMATRIX getTransform() const noexcept;
	const DirectX::XMFLOAT3& getPosition() const noexcept;
	DirectX::XMFLOAT3& getPosition();
	DirectX::XMFLOAT3 getRotation() const noexcept;
	//	\function	getDirection	||	\date	2022/08/30 23:11
	//	\brief	returns the vector describing the camera's direction aka "lookVector"
	DirectX::XMVECTOR getDirection() const noexcept;
	DirectX::XMVECTOR getRight() const noexcept;
	DirectX::XMVECTOR getUp() const noexcept;
	float getFovRadians() const noexcept;
	const std::string& getName() const noexcept;
	std::vector<DirectX::XMFLOAT4> getFrustumPlanes() const noexcept;
	void displayImguiWidgets( Graphics &gfx ) noexcept;
	void updateDimensions( Graphics &gfx );
	void setTethered( const bool bTethered ) cond_noex;
private:
	DirectX::XMMATRIX getPositionMatrix() const noexcept;
	DirectX::XMMATRIX getRotationMatrix() const noexcept;
	//	\function	getTarget	||	\date	2024/04/28 13:19
	//	\brief	target is the position of the camera's focus/focal point
	//			ofc the camera doesn't look at a position, it looks along a direction; any points along that direction will do to compute the view-matrix
	DirectX::XMVECTOR getTarget() const noexcept;
	void updateCameraFrustum( Graphics &gfx );
};

using Projector = Camera;