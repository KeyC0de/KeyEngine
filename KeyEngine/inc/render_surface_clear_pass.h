#pragma once

#include "pass.h"


class IRenderSurface;

namespace ren
{

class RenderSurfaceClearPass
	: public IPass
{
	std::shared_ptr<IRenderSurface> m_pRenderSurface;
public:
	RenderSurfaceClearPass( const std::string &name );

	void run( Graphics &gfx ) const cond_noex override;
	void reset() cond_noex override;
};


}