#pragma once

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "render_queue_pass.h"


class Graphics;
class Camera;
class CubeTextureDS;
class IDepthStencilView;
class LightVSCB;

namespace ren
{

class ShadowPass
	: public RenderQueuePass
{
	static inline unsigned m_shadowMapResolution = 1024;
	const Camera *m_pShadowCamera = nullptr;
	std::shared_ptr<LightVSCB> m_pLightVcb;
	std::shared_ptr<CubeTextureDS> m_pDsvCubemap;
	DirectX::XMFLOAT4X4 m_cameraShadowProjectionMatrix;
	std::vector<DirectX::XMFLOAT3> m_cameraDirections{6};
	std::vector<DirectX::XMFLOAT3> m_cameraUps{6};
public:
	ShadowPass( Graphics &gph, const std::string &name,
		unsigned shadowMapRez = m_shadowMapResolution );

	//===================================================
	//	\function	update
	//	\brief  update the light's -camera- view Proj Matrix for projective texture shadow cube mapping
	//				then render the depth buffer to texture 6 times
	//	\date	2021/10/18 23:55
	void run( Graphics &gph ) const cond_noex override;
	void setShadowCamera( const Camera &cam ) noexcept;
	void dumpShadowMap( Graphics &gph, const std::string &path ) const;
	static const unsigned getResolution() noexcept;
private:
	void setDepthStencilView( std::shared_ptr<IDepthStencilView> ds ) const;
};


}