#pragma once

#include <string>
#include <DirectXMath.h>
#include "model.h"
#include "rendering_channel.h"


class Graphics;

namespace ren
{
	class Renderer;
}

class Camera
{
	float m_translationSpeed;
	float m_rotationSpeed;
	float m_nearZ;
	float m_farZ;
	float m_NearZDefault;
	float m_FarZDefault;
	float m_width;
	float m_height;
	float m_widthDefault;
	float m_heightDefault;
	float m_aspectRatio;
	float m_fovRadians;
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_posPrev;
	DirectX::XMFLOAT3 m_posDefault;
	float m_pitch;	// in radians
	float m_pitchPrev;
	float m_pitchDefault;
	float m_yawDefault;
	float m_yaw;	// in radians
	float m_yawPrev;
	// tethered flag indicates whether the Camera's position is bound to another object
	// (eg shadow casting light), st when the object moves the camera moves along with it
	bool m_bTethered;
	bool m_bShowWidget = false;
	bool m_bShowFrustum = false;
	Model m_cameraWidget;
	Model m_cameraFrustum;
	std::string m_name;
public:
	static DirectX::XMMATRIX getShadowOrthographicMatrix( const unsigned w, const unsigned h, const float shadowCamFarZ, const float shadowCamNearZ = 1.0f ) noexcept;
	static DirectX::XMMATRIX getShadowProjectionMatrix( const float shadowCamFarZ, const float shadowCamNearZ = 1.0f ) noexcept;
public:
	Camera( Graphics &gfx, const float fovDegrees = 90.0f, const DirectX::XMFLOAT3 &posDefault = {0.0f, 0.0f, 0.0f}, const float pitchDegDefault = 0.0f, const float yawDegDefault = 0.0f, const bool bTethered = false, const float nearZ = 0.5f, const float farZ = 200.0f, const DirectX::XMFLOAT4 camWidgetColor = {0.2f, 0.2f, 0.6f, 1.0f}, const DirectX::XMFLOAT4 camFrustumColor = {0.6f, 0.2f, 0.2f, 1.0f}, const float translationSpeed = 16.0f, const float rotationSpeed = 0.096f ) noexcept;

	void update( const float dt, const float renderFrameInterpolation, const bool bEnableSmoothMovement = false ) cond_noex;
	void render( const size_t channel = rch::all ) const cond_noex;
	void connectMaterialsToRenderer( ren::Renderer &ren );
	void makeActive( Graphics &gfx, const bool bOrthographic ) const;
	void resetToDefault( Graphics &gfx ) noexcept;
	DirectX::XMMATRIX getViewMatrix() const noexcept;
	DirectX::XMMATRIX getReflectionViewMatrix( const DirectX::XMVECTOR &mirrorPlane ) const noexcept;
	DirectX::XMMATRIX getOrthographicProjectionMatrix( const unsigned width, const unsigned height ) const noexcept;
	DirectX::XMMATRIX getPerspectiveProjectionMatrix() const noexcept;
	//	\function	getDirection	||	\date	2022/08/30 23:11
	//	\brief	returns the vector describing the camera's direction aka "lookVector"
	DirectX::XMVECTOR getDirection() const noexcept;
	DirectX::XMVECTOR getRight() const noexcept;
	DirectX::XMVECTOR getUp() const noexcept;
	float getFovRadians() const noexcept;
	const std::string& getName() const noexcept;
	std::vector<DirectX::XMFLOAT4> getFrustumPlanes() const noexcept;
	void displayImguiWidgets( Graphics &gfx ) noexcept;
	void onWindowResize( Graphics &gfx );
	void setTethered( const bool bTethered ) cond_noex;
	void setRotationSpeed( const float rotationSpeed ) noexcept;
	float getRotationSpeed() const noexcept;
	// transformation functions
	// rotate arguments is mouse dx, dy delta values
	void setRotation( const DirectX::XMFLOAT3 &rot ) noexcept;
	void rotateRel( const float dx, const float dy ) noexcept;
	void translateRel( DirectX::XMFLOAT3 translation ) noexcept;
	void setTranslation( const DirectX::XMFLOAT3 &pos ) noexcept;
	//	\function	getTransform	||	\date	2024/04/28 13:10
	//	\brief	gets the camera matrix
	const DirectX::XMFLOAT3& getPosition() const noexcept;
	DirectX::XMFLOAT3& getPosition();
	DirectX::XMFLOAT3 getRotation() const noexcept;
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
