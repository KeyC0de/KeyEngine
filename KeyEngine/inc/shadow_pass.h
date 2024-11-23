#pragma once

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "render_queue_pass.h"
#include "constant_buffer.h"
#include "light_source.h"


class Graphics;
class Camera;
class TextureArrayOffscreenDS;
class CubeTextureArrayOffscreenDS;
class IDepthStencilView;

namespace ren
{

///=============================================================
/// \class	ShadowPass
/// \author	KeyC0de
/// \date	2022/10/03 10:30
/// \brief	An offscreen Pass - output only pass DSV
///=============================================================
class ShadowPass
	: public RenderQueuePass
{
	static constexpr inline unsigned s_maxShadowCastingLights = 16u;
	static constexpr inline unsigned s_globalsVscbSlot = 1u;
	static constexpr inline unsigned s_globalsPscbSlot = 1u;
	static constexpr inline unsigned s_lightArrayVertexShaderCBSlot = 2u;
	static constexpr inline unsigned s_lightArrayPixelShaderCBSlot = 2u;
	static constexpr inline unsigned s_shadowMapArraySlot = 3u;
	static constexpr inline unsigned s_shadowCubeMapArraySlot = 4u;
	static inline unsigned s_shadowMapResolution = 1024u;

	struct GlobalsVSCB
	{
		float cb_time;
		int cb_nLights;
		int cb_nPointLights;
		float cb_globalsPadding;
	};
	VertexShaderConstantBuffer<GlobalsVSCB> m_globalsVscb;

	struct GlobalsPSCB
	{
		float cb_time;
		int cb_nLights;
		int cb_nPointLights;
		float cb_globalsPadding;
	};
	PixelShaderConstantBuffer<GlobalsPSCB> m_globalsPscb;

	using LightsVSCB = ILightSource::LightVSCB[s_maxShadowCastingLights];
	VertexShaderConstantBuffer<LightsVSCB> m_vscb;
	using LightsPSCB = ILightSource::LightPSCB[s_maxShadowCastingLights];
	PixelShaderConstantBuffer<LightsPSCB> m_pscb;

	std::vector<DirectX::XMVECTOR> m_cameraDirections{6};
	std::vector<DirectX::XMVECTOR> m_cameraUps{6};
	std::vector<ILightSource*> m_shadowCastingLights;
	std::shared_ptr<TextureArrayOffscreenDS> m_pOffscreenDsvMapArray;			// shadow maps for Directional/Spot lights
	std::shared_ptr<CubeTextureArrayOffscreenDS> m_pOffscreenDsvCubemapArray;	// shadow cube maps for Point lights
public:
	static unsigned getResolution() noexcept;
public:
	ShadowPass( Graphics &gfx, const std::string &name, const unsigned shadowMapRez = s_shadowMapResolution );

	/// \brief	update the light's -camera- view Proj Matrix for projective texture shadow cube mapping
	//				then render the depth buffer to texture 6 times
	void run( Graphics &gfx ) const cond_noex override;
	/// \brief	populate shadow casting lights for this frame and setup their offscreen shadow maps for rendering into
	void bindShadowCastingLights( Graphics &gfx, const std::vector<ILightSource*> &shadowCastingLights );
	/// \brief	currently only dumping shadow map of the first registered shadow casting light
	void dumpShadowMap( Graphics &gfx, const std::string &path ) const;
	void dumpShadowCubeMap( Graphics &gfx, const std::string &path ) const;
private:
	void bindGlobalCBs( Graphics &gfx ) cond_noex;
	void bindLightCBs( Graphics &gfx ) cond_noex;
};


}//namespace ren