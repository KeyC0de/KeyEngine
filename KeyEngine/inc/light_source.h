#pragma once

#include "graphics.h"
#include "sphere.h"
#include "constant_buffer.h"


namespace ren
{
	class Renderer;
}
class Camera;

class DirectionalLight
{
	static constexpr unsigned s_directionalLightPscbSlot = 1u;
	struct PSCB final	// DirectionalLightPSCB
	{
		alignas(16) DirectX::XMFLOAT3 dir;	// directionalLightDirViewSpace in HLSL
		alignas(16) DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 lightColor;
		alignas(16) float intensity;
	};
};

class PointLight
{
	std::string m_name;
	static constexpr unsigned s_pointLightPscbSlot = 2u;
	// or __declspec(align(16))
	struct PSCB final	// PointLightPSCB
	{
		alignas(16) DirectX::XMFLOAT3 pos;	// pointLightPosViewSpace in HLSL
		alignas(16) DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 lightColor;
		float intensity;
		float attConstant;
		float attLinear;
		float attQuadratic;
	};
	PSCB m_pscbData;
	PSCB m_pscbHomeData;
	mutable Sphere m_sphereMesh;
	mutable PixelShaderConstantBuffer<PSCB> m_pscb;
	bool m_bCastingShadows;
	bool m_bShowMesh;
	std::shared_ptr<Camera> m_pCameraForShadowing;
public:
	PointLight( Graphics &gph, const DirectX::XMFLOAT3 &pos = {8.0f, 8.0f, 2.f}, const DirectX::XMFLOAT3 &col = {1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const bool bShowMesh = true, const float radius = 0.5f );
	
	void update( Graphics &gph, const float dt, const DirectX::XMMATRIX &activeCameraViewMat ) const noexcept;
	void render( const size_t channels ) const cond_noex;
	void connectEffectsToRenderer( ren::Renderer &r );
	void setIntensity( const float newIntensity ) noexcept;
	void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept;
	void displayImguiWidgets() noexcept;
	void resetToDefault() noexcept;
	std::shared_ptr<Camera> shareCamera() const noexcept;
	bool isCastingShadows() const noexcept;
	const std::string& getName() const noexcept;
};

class SpotLight
{
	static constexpr unsigned s_spotLightPscbSlot = 3u;
	struct PSCB final	// SpotLightPSCB
	{
		alignas(16) DirectX::XMFLOAT3 pos;	// spotLightPosViewSpace in HLSL
		alignas(16) DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 lightColor;
		float intensity;
		float attConstant;
		float attLinear;
		float attQuadratic;
		alignas(16) DirectX::XMFLOAT3 dir;	// spotLightDirViewSpace in HLSL, probably not needed computed in HLSL much like PointLight
		alignas(16) float coneAngle;
	};
};