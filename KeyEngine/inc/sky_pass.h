#pragma once

#include <vector>
#include "bindable_pass.h"


class Graphics;
class Camera;
class VertexBuffer;
class IndexBuffer;

namespace ren
{

class SkyPass
	: public IBindablePass
{
	bool m_bUseSphere = true;
	const Camera *m_pMainCamera = nullptr;
	std::shared_ptr<VertexBuffer> m_pCubeVb;
	std::shared_ptr<IndexBuffer> m_pCubeIb;
	unsigned m_nCubeIndices;
	std::shared_ptr<VertexBuffer> m_pSphereVb;
	std::shared_ptr<IndexBuffer> m_pSphereIb;
	unsigned m_nSphereIndices;
public:
	SkyPass( Graphics &gph, const std::string &name );

	void setMainCamera( const Camera &cam ) noexcept;
	void run( Graphics &gph ) const cond_noex override;
	void reset() cond_noex override;
	void displayImguiWidgets();
};


}