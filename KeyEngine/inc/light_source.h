#pragma once

#include <variant>
#include <DirectXMath.h>
#include "model.h"


namespace ren
{
class Renderer;
class ShadowPass;
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

////////////////////////////////////////////////////////////////////////////////////////////////////
class ILightSource
{
	friend class ren::ShadowPass;

	LightSourceType m_type;
protected:
	float m_shadowCamFarZ;
	std::string m_name;
	bool m_bShowMesh;
	bool m_bShadowCasting;
	Model m_lightMesh;

	struct LightVSCB
	{
		DirectX::XMMATRIX cb_lightMatrix;
	};
	LightVSCB m_vscbData;	// used for shadowing
	std::unique_ptr<Camera> m_pCameraForShadowing;	// this camera is not shared in CameraManager

	// HLSL packing rules : an element is not allowed to straddle a 4D vector boundary (this is allowed in C++, but we must conform to our weakest link = HLSL)
	struct LightPSCB
	{
		int cb_lightType;
		int cb_bShadowCasting;
		float padding[2];
		alignas(16) DirectX::XMFLOAT3 cb_lightPosViewSpace;	// represents direction for Directional Lights
		alignas(16) DirectX::XMFLOAT3 cb_ambientColor;
		alignas(16) DirectX::XMFLOAT3 cb_lightColor;
		float cb_intensity;
		float cb_attConstant;								// unused for Directional Lights
		float cb_attLinear;									// unused for Directional Lights
		float cb_attQuadratic;								// unused for Directional Lights
		DirectX::XMFLOAT3 cb_spotLightDirViewSpace;			// only used for Spot Lights
		float cb_coneAngle;									// only used for Spot Lights
	};
	LightPSCB m_pscbData;
	LightPSCB m_pscbDataToBind;
public:
	ILightSource( Graphics &gfx, const LightSourceType type, Model model, const std::variant<DirectX::XMFLOAT4,std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const bool bShowMesh = true, const DirectX::XMFLOAT3 &rotDegOrDirectionVector = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &pos = DirectX::XMFLOAT3{8.0f, 8.0f, 2.f}, const float intensity = 1.0f, const float fovDeg = 0.0f, const float shadowCamFarZ = 100.0f );
	virtual ~ILightSource() noexcept = default;

	/// \brief	update the "model" side part of the light
	void update( const float dt, const float lerpBetweenFrames, const bool bEnableSmoothMovement = false ) cond_noex;
	/// \brief	render the "model" side part of the light
	void render( const size_t channels ) const cond_noex;
	void connectMaterialsToRenderer( ren::Renderer &r );
	virtual void displayImguiWidgets() noexcept = 0;
	virtual void setIntensity( const float newIntensity ) noexcept = 0;
	virtual void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept = 0;
	void setRotation( const DirectX::XMFLOAT3 &rot );
	void setTranslation( const DirectX::XMFLOAT3 &pos );
	LightSourceType getType() const noexcept;
	unsigned getSlot() const noexcept;
	std::string getName() const noexcept;
	bool isCastingShadows() const noexcept;
	bool isFrustumCulled() const noexcept;
	DirectX::XMFLOAT3 getRotation() const noexcept;
	virtual DirectX::XMFLOAT3 getPosition() const noexcept = 0;
	Camera* getShadowCamera() const;
	float getShadowCameraFarZ() const noexcept;
private:
	LightVSCB getVscbData() noexcept;
	LightPSCB getPscbData() noexcept;
	void populateCBData( Graphics &gfx ) cond_noex;
	virtual void populateVscbData( Graphics &gfx ) cond_noex = 0;
	virtual void populatePscbData( Graphics &gfx ) cond_noex = 0;
};

class DirectionalLight final
	: public ILightSource
{
public:
	DirectionalLight( Graphics &gfx, const float radiusScale = 1.0f, const DirectX::XMFLOAT3 &directionVector = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &lightMeshPos = {8.0f, 8.0f, 2.f}, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const bool bShowMesh = true, const float intensity = 1.0f, const float shadowCamNearZ = 1.0f, const float shadowCamFarZ = 2000.0f );

	void displayImguiWidgets() noexcept override;
	void setIntensity( const float newIntensity ) noexcept override;
	void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept override;
	virtual DirectX::XMFLOAT3 getPosition() const noexcept override;
private:
	virtual void populateVscbData( Graphics &gfx ) cond_noex override;
	virtual void populatePscbData( Graphics &gfx ) cond_noex override;
};

class SpotLight final
	: public ILightSource
{
public:
	SpotLight( Graphics &gfx, const float radiusScale = 1.0f, const DirectX::XMFLOAT3 &directionVector = {0.0f, 0.0f, 1.0f}, const DirectX::XMFLOAT3 &pos = {8.0f, 8.0f, 2.f}, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const bool bShowMesh = true, const float intensity = 1.0f, const float fovConeAngle = 60.0f, const float shadowCamNearZ = 0.5f, const float shadowCamFarZ = 80.0f );

	void displayImguiWidgets() noexcept override;
	void setIntensity( const float newIntensity ) noexcept override;
	void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept override;
	virtual DirectX::XMFLOAT3 getPosition() const noexcept override;
private:
	virtual void populateVscbData( Graphics &gfx ) cond_noex override;
	virtual void populatePscbData( Graphics &gfx ) cond_noex override;
};

class PointLight final
	: public ILightSource
{
public:
	PointLight( Graphics &gfx, const float radiusScale = 0.5f, const DirectX::XMFLOAT3 &lightMeshRotationDeg = {0.0f, 0.0f, 0.f}, const DirectX::XMFLOAT3 &pos = {8.0f, 8.0f, 2.f}, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const bool bShowMesh = true, const float intensity = 1.0f, const float shadowCamNearZ = 1.0f, const float shadowCamFarZ = 100.0f );

	void displayImguiWidgets() noexcept override;
	void setIntensity( const float newIntensity ) noexcept override;
	void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept override;
	virtual DirectX::XMFLOAT3 getPosition() const noexcept override;
private:
	virtual void populateVscbData( Graphics &gfx ) cond_noex override;
	virtual void populatePscbData( Graphics &gfx ) cond_noex override;
};