#pragma once

#include <vector>
#include "bindable_pass.h"


class Graphics;
class VertexBuffer;
class IndexBuffer;

namespace ren
{

class SkyPass
	: public IBindablePass
{
	static constexpr inline const char *s_cubeGeometryTag = "$cube_skybox";
	static constexpr inline const char *s_sphereGeometryTag = "$sphere_skybox";

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
};


}//namespace ren