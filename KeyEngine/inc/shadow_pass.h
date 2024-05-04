#pragma once

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "render_queue_pass.h"


class Graphics;
class Camera;
class CubeTextureOffscreenDS;
class IDepthStencilView;
class PointLightSourceShadowVSCB;

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
	const Camera *m_pCameraForShadowing = nullptr;
	std::shared_ptr<PointLightSourceShadowVSCB> m_pLightVcb;	// #TODO: switch this to store a vector of shadow casting light sources and have the LightSourceShadowVSCB inside the light source
	std::shared_ptr<CubeTextureOffscreenDS> m_pOffscreenDsvCubemap;
	DirectX::XMFLOAT4X4 m_cameraShadowProjectionMatrix;
	std::vector<DirectX::XMFLOAT3> m_cameraDirections{6};
	std::vector<DirectX::XMFLOAT3> m_cameraUps{6};
public:
	static unsigned getResolution() noexcept;
public:
	ShadowPass( Graphics &gfx, const std::string &name, const unsigned shadowMapRez = s_shadowMapResolution );

	//	\function	update	||	\date	2021/10/18 23:55
	//	\brief  update the light's -camera- view Proj Matrix for projective texture shadow cube mapping
	//				then render the depth buffer to texture 6 times
	void run( Graphics &gfx ) const cond_noex override;
	void setShadowCamera( const Camera &cam, const bool bEnable ) noexcept;
	void dumpShadowMap( Graphics &gfx, const std::string &path ) const;
};


}