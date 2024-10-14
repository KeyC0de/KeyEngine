#pragma once

#include <variant>
#include <DirectXMath.h>
#include "model.h"
#include "constant_buffer.h"


namespace ren
{
	class Renderer;
}

class Graphics;
class Camera;
class ILightSource;

enum LightSourceType : unsigned
{
	Directional = 1u,
	Spot,
	Point,
};

//=============================================================
//	\class	LightSourceVSCB
//	\author	KeyC0de
//	\date	2022/02/19 22:35
//	\brief	VSCB for a light source for the purposes of shadowing, used to pass its location to GPU/HLSL(shadowing_vs.hlsli)
//=============================================================
class LightSourceVSCB
	: public IBindable
{
	friend class ILightSource;

	struct LightVSCB
	{
		DirectX::XMMATRIX cb_lightMatrix;
	};
	std::unique_ptr<VertexShaderConstantBuffer<LightVSCB>> m_pVscb;
	std::unique_ptr<Camera> m_pCameraForShadowing;	// these cameras are not shared in CameraManager
	LightSourceType m_type;
	float m_shadowCamFarZ;
public:
	LightSourceVSCB( Graphics &gfx, const LightSourceType type, const float shadowCamFovDegrees = 90.0f, const DirectX::XMFLOAT3 &shadowCamPos = DirectX::XMFLOAT3{0.0f, 0.0f, 0.0f}, const float shadowCamPitchDeg = 0.0f, const float shadowCamYawDeg = 0.0f, const float shadowCamNearZ = 1.0f, const float shadowCamFarZ = 100.0f, const DirectX::XMFLOAT3 &direction = DirectX::XMFLOAT3{0.0f, 0.0f, 0.0f} );

	void bind( Graphics &gfx ) cond_noex override;
	void update( Graphics &gfx ) cond_noex;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class ILightSource
{
	LightSourceType m_type;
	float m_shadowCamFarZ;
protected:
	std::string m_name;
	bool m_bShowMesh;
	bool m_bCastingShadows;
	DirectX::XMFLOAT3 m_rot;		// {x = pitch, y = yaw, y = roll} - in radians
	Model m_lightMesh;
	std::shared_ptr<LightSourceVSCB> m_lightVscb;

	struct LightPSCB
	{
		int cb_lightType;
		int cb_bShadowCasting;
		float padding[2];
		alignas(16) DirectX::XMFLOAT3 cb_lightPosViewSpace;	// represents direction for Directional Lights
		alignas(16) DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 lightColor;
		float intensity;// HLSL packing rules : an element is not allowed to straddle a 4D vector boundary (this is allowed in C++, but we must conform to our weakest link = HLSL)
		float attConstant;									// unused for Directional Lights
		float attLinear;									// unused for Directional Lights
		float attQuadratic;									// unused for Directional Lights
		DirectX::XMFLOAT3 cb_spotLightDirViewSpace;			// only used for Spot Lights
		float cb_coneAngle;									// only used for Spot Lights
	};
	LightPSCB m_pscbData;
	PixelShaderConstantBuffer<LightPSCB> m_pscb;
public:
	ILightSource( Graphics &gfx, const LightSourceType type, Model model, const std::variant<DirectX::XMFLOAT4,std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const bool bShowMesh = true, const DirectX::XMFLOAT3 &initialRotDeg = {0.0f, 0.0f, 0.f}, const DirectX::XMFLOAT3 &pos = DirectX::XMFLOAT3{8.0f, 8.0f, 2.f}, const float intensity = 1.0f, const DirectX::XMFLOAT3 &direction = DirectX::XMFLOAT3{0.0f, 0.0f, 0.0f}, const float fovDeg = 0.0f, const float shadowCamNearZ = 1.0f, const float shadowCamFarZ = 100.0f );
	virtual ~ILightSource() noexcept = default;

	virtual void update( Graphics &gfx, const float dt, const float lerpBetweenFrames, const bool bEnableSmoothMovement = false ) cond_noex;
	void render( const size_t channels ) const cond_noex;
	void updateVscb( Graphics &gfx ) const cond_noex;
	void bindVscb( Graphics &gfx ) const cond_noex;
	void connectMaterialsToRenderer( ren::Renderer &r );
	virtual void displayImguiWidgets() noexcept = 0;
	virtual void setIntensity( const float newIntensity ) noexcept = 0;
	virtual void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept = 0;
	Camera* getShadowCamera() const;
	std::shared_ptr<LightSourceVSCB> shareVscb() const noexcept;
	LightSourceType getType() const noexcept;
	unsigned getSlot() const noexcept;
	bool isCastingShadows() const noexcept;
	bool isFrustumCulled() const noexcept;
	std::string getName() const noexcept;
	void setRotation( const DirectX::XMFLOAT3 &rot );
	void setTranslation( const DirectX::XMFLOAT3 &pos );
	DirectX::XMFLOAT3 getRotation() const noexcept;
	virtual DirectX::XMFLOAT3 getPosition() const noexcept = 0;
	float getShadowCameraFarZ() const noexcept;
	DirectX::XMMATRIX getShadowCameraProjectionMatrix( Graphics &gfx ) cond_noex;
protected:
	Camera* accessShadowCamera();
};

class DirectionalLight final
	: public ILightSource
{
public:
	DirectionalLight( Graphics &gfx, const float radiusScale = 1.0f, const DirectX::XMFLOAT3 &initialRotDeg = {0.0f, 0.0f, 0.f}, const DirectX::XMFLOAT3 &pos = {8.0f, 8.0f, 2.f}, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const bool bShowMesh = true, const float intensity = 1.0f, const DirectX::XMFLOAT3 &direction = DirectX::XMFLOAT3{0.0f, 0.0f, 0.0f}, const float shadowCamNearZ = 1.0f, const float shadowCamFarZ = 2000.0f );

	void update( Graphics &gfx, const float dt, const float lerpBetweenFrames, const bool bEnableSmoothMovement = false ) cond_noex override;
	void displayImguiWidgets() noexcept override;
	void setIntensity( const float newIntensity ) noexcept override;
	void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept override;
	virtual DirectX::XMFLOAT3 getPosition() const noexcept override;
};

class SpotLight final
	: public ILightSource
{
public:
	SpotLight( Graphics &gfx, const float radiusScale = 1.0f, const DirectX::XMFLOAT3 &initialRotDeg = {0.0f, 0.0f, 0.f}, const DirectX::XMFLOAT3 &pos = {8.0f, 8.0f, 2.f}, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const bool bShowMesh = true, const float intensity = 1.0f, const DirectX::XMFLOAT3 &direction = DirectX::XMFLOAT3{0.0f, 0.0f, 1.0f}, const float fovConeAngle = 60.0f, const float shadowCamNearZ = 0.5f, const float shadowCamFarZ = 80.0f );

	void update( Graphics &gfx, const float dt, const float lerpBetweenFrames, const bool bEnableSmoothMovement = false ) cond_noex override;
	void displayImguiWidgets() noexcept override;
	void setIntensity( const float newIntensity ) noexcept override;
	void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept override;
	virtual DirectX::XMFLOAT3 getPosition() const noexcept override;
};

class PointLight final
	: public ILightSource
{
public:
	PointLight( Graphics &gfx, const float radiusScale = 0.5f, const DirectX::XMFLOAT3 &initialRotDeg = {0.0f, 0.0f, 0.f}, const DirectX::XMFLOAT3 &pos = {8.0f, 8.0f, 2.f}, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const bool bShowMesh = true, const float intensity = 1.0f, const float shadowCamNearZ = 1.0f, const float shadowCamFarZ = 100.0f );
	
	void update( Graphics &gfx, const float dt, const float lerpBetweenFrames, const bool bEnableSmoothMovement = false ) cond_noex override;
	void displayImguiWidgets() noexcept override;
	void setIntensity( const float newIntensity ) noexcept override;
	void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept override;
	virtual DirectX::XMFLOAT3 getPosition() const noexcept override;
};