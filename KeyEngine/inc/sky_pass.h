#pragma once

#include "bindable_pass.h"
#include "constant_buffer.h"


class Graphics;
class VertexBuffer;
class IndexBuffer;

namespace ren
{

///=============================================================
/// \class	SkyPass
/// \author	KeyC0de
/// \date	2021/12/01 13:39
/// \brief	SkyPass inherits from IBindablePass
/// \brief	no need to make SkyPass inherit from RenderQueuePass to add a Job just for a single "special" object (the SkyDome)
///=============================================================
class SkyPass
	: public IBindablePass
{
	static constexpr inline const char *s_cubeGeometryTag = "$cube_skybox";
	static constexpr inline const char *s_sphereGeometryTag = "$sphere_skybox";

	struct SkyTransform
	{
		DirectX::XMMATRIX viewProjection;
	};
	VertexShaderConstantBuffer<SkyTransform> m_skyVscb;

	unsigned m_nCubeIndices;
	unsigned m_nSphereIndices;
	std::shared_ptr<VertexBuffer> m_pCubeVb;
	std::shared_ptr<IndexBuffer> m_pCubeIb;
	std::shared_ptr<VertexBuffer> m_pSphereVb;
	std::shared_ptr<IndexBuffer> m_pSphereIb;
	bool m_bUseSphere;
public:
	SkyPass( Graphics &gfx, const std::string &name, const bool useSphere );

	void run( Graphics &gfx ) const cond_noex override;
	void reset() cond_noex override;
	void displayImguiWidgets() noexcept;
private:
	void bindSkyVSCB( Graphics &gfx ) cond_noex;
	SkyTransform getTransform( Graphics &gfx ) cond_noex;
};


}//namespace ren