#pragma once

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "render_queue_pass.h"
#include "constant_buffer.h"


class Graphics;
class Camera;
class TextureArrayOffscreenDS;
class CubeTextureArrayOffscreenDS;
class IDepthStencilView;
class ILightSource;

namespace ren
{

//=============================================================
//	\class	ShadowPass
//	\author	KeyC0de
//	\date	2022/10/03 10:30
//	\brief	An offscreen Pass - output only pass DSV
//=============================================================
class ShadowPass
	: public RenderQueuePass
{
	static inline unsigned s_shadowMapResolution = 1024u;
	static inline unsigned s_globalsVscbSlot = 1u;
	static inline unsigned s_globalsPscbSlot = 1u;
	static inline unsigned s_shadowMapArraySlot = 3u;
	static inline unsigned s_shadowCubeMapArraySlot = 4u;

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

	std::vector<DirectX::XMVECTOR> m_cameraDirections{6};
	std::vector<DirectX::XMVECTOR> m_cameraUps{6};
	std::vector<ILightSource*> m_shadowCastingLights;
	std::shared_ptr<TextureArrayOffscreenDS> m_pOffscreenDsvMapArray;			// shadow maps for Directional/Spot lights
	std::shared_ptr<CubeTextureArrayOffscreenDS> m_pOffscreenDsvCubemapArray;	// shadow cube maps for Point lights
public:
	static unsigned getResolution() noexcept;
public:
	ShadowPass( Graphics &gfx, const std::string &name, const unsigned shadowMapRez = s_shadowMapResolution );

	//	\function	update	||	\date	2021/10/18 23:55
	//	\brief	update the light's -camera- view Proj Matrix for projective texture shadow cube mapping
	//				then render the depth buffer to texture 6 times
	void run( Graphics &gfx ) const cond_noex override;
	void bindGlobalConstantBuffers( Graphics &gfx, const unsigned nShadowCastingLights, const unsigned nShadowCastingPointLights ) cond_noex;
	//	\function	setShadowCastingLights	||	\date	2024/09/07 9:38
	//	\brief	populate shadow casting lights for this frame and setup their offscreen shadow maps for rendering into
	void setShadowCastingLights( Graphics &gfx, const std::vector<ILightSource*> &shadowCastingLights );
	// currently only dumping shadow map of the first registered shadow casting light
	void dumpShadowMap( Graphics &gfx, const std::string &path ) const;
	void dumpShadowCubeMap( Graphics &gfx, const std::string &path ) const;
};


}//namespace ren