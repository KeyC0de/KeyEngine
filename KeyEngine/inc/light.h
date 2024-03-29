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
	bool m_bShadowCasting;
	std::shared_ptr<Camera> m_pShadowCamera;
public:
	PointLight( Graphics &gph, const DirectX::XMFLOAT3 &pos = {8.0f, 8.0f, 2.f}, const DirectX::XMFLOAT3 &col = {1.0f, 1.0f, 1.0f}, const bool bShadowCasting = true, const float radius = 0.5f );

	const std::string& getName() const noexcept;
	void setIntensity( const float newIntensity ) noexcept;
	void setColor( const DirectX::XMFLOAT3 &diffuseColor ) noexcept;
	bool isCastingShadows() const noexcept;
	void displayImguiWidgets() noexcept;
	void resetToDefault() noexcept;
	void update( Graphics &gph, const float dt, const DirectX::XMMATRIX &activeCameraViewMat ) const noexcept;
	void render( const size_t channels ) const cond_noex;
	void connectEffectsToRenderer( ren::Renderer &r );
	std::shared_ptr<Camera> shareCamera() const noexcept;
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
		alignas(16) DirectX::XMFLOAT3 dir;	// spotLightDirViewSpace in HLSL
		alignas(16) float coneAngle;
	};
};