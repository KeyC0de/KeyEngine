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

class ILightSource
{
protected:
	std::string m_name;
	bool m_bShowMesh;
	bool m_bCastingShadows;
	DirectX::XMFLOAT3 m_rot;		// {x = pitch, y = yaw, y = roll} - in radians
	Model m_lightMesh;
	std::shared_ptr<Camera> m_pCameraForShadowing;
public:
	ILightSource( Model model, const bool bShadowCasting = true, const bool bShowMesh = true, const DirectX::XMFLOAT3 &initialRotDeg = {0.0f, 0.0f, 0.f} );
	virtual ~ILightSource() noexcept = default;

	void render( const size_t channels ) const cond_noex;
	void connectMaterialsToRenderer( ren::Renderer &r );
	std::shared_ptr<Camera> shareCamera() const noexcept;
	bool isCastingShadows() const noexcept;
	std::string getName() const noexcept;

	virtual void update( Graphics &gfx, const float dt, const float lerpBetweenFrames, const bool bEnableSmoothMovement = false ) cond_noex;
	virtual void displayImguiWidgets() noexcept = 0;
	virtual void setIntensity( const float newIntensity ) noexcept = 0;
	virtual void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept = 0;
};

class DirectionalLight final
	: public ILightSource
{
	static constexpr unsigned s_directionalLightPscbSlot = 1u;

	struct DirectionalLightPSCB
	{
		alignas(16) DirectX::XMFLOAT3 directionalLightDirViewSpace;
		alignas(16) DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 lightColor;
		float intensity;
	};
	DirectionalLightPSCB m_pscbData;
	PixelShaderConstantBuffer<DirectionalLightPSCB> m_pscb;
public:
	DirectionalLight( Graphics &gfx, const float radiusScale = 1.0f, const DirectX::XMFLOAT3 &initialRotDeg = {0.0f, 0.0f, 0.f}, const DirectX::XMFLOAT3 &pos = {8.0f, 8.0f, 2.f}, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const bool bShowMesh = true );

	void update( Graphics &gfx, const float dt, const float lerpBetweenFrames, const bool bEnableSmoothMovement = false ) cond_noex override;
	void displayImguiWidgets() noexcept override;
	void setIntensity( const float newIntensity ) noexcept override;
	void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept override;
};

class PointLight final
	: public ILightSource
{
	static constexpr unsigned s_pointLightPscbSlot = 2u;

	struct /* __declspec(align(16)) */ PointLightPSCB
	{
		alignas(16) DirectX::XMFLOAT3 pointLightPosViewSpace;
		alignas(16) DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 lightColor;
		float intensity;
		float attConstant;
		float attLinear;
		float attQuadratic;
	};
	PointLightPSCB m_pscbData;
	PixelShaderConstantBuffer<PointLightPSCB> m_pscb;
public:
	PointLight( Graphics &gfx, const float radiusScale = 0.5f, const DirectX::XMFLOAT3 &initialRotDeg = {0.0f, 0.0f, 0.f}, const DirectX::XMFLOAT3 &pos = {8.0f, 8.0f, 2.f}, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const bool bShowMesh = true );
	
	void update( Graphics &gfx, const float dt, const float lerpBetweenFrames, const bool bEnableSmoothMovement = false ) cond_noex override;
	void displayImguiWidgets() noexcept override;
	void setIntensity( const float newIntensity ) noexcept override;
	void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept override;
};

class SpotLight final
	: public ILightSource
{
	static constexpr unsigned s_spotLightPscbSlot = 3u;

	struct SpotLightPSCB
	{
		alignas(16) DirectX::XMFLOAT3 spotLightPosViewSpace;
		alignas(16) DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 lightColor;
		float intensity;
		float attConstant;
		float attLinear;
		float attQuadratic;
		DirectX::XMFLOAT3 spotLightDirViewSpace;
		float coneAngle;
	};
	SpotLightPSCB m_pscbData;
	PixelShaderConstantBuffer<SpotLightPSCB> m_pscb;
public:
	SpotLight( Graphics &gfx, const float radiusScale = 1.0f, const DirectX::XMFLOAT3 &initialRotDeg = {0.0f, 0.0f, 0.f}, const DirectX::XMFLOAT3 &pos = {8.0f, 8.0f, 2.f}, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, const float coneAngle = 60.0f, const bool bShadowCasting = true, const bool bShowMesh = true );

	void update( Graphics &gfx, const float dt, const float lerpBetweenFrames, const bool bEnableSmoothMovement = false ) cond_noex override;
	void displayImguiWidgets() noexcept override;
	void setIntensity( const float newIntensity ) noexcept override;
	void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept override;
};
