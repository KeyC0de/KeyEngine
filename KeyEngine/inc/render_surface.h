#pragma once

#include <array>


class Graphics;

class IRenderSurface
{
public:
	virtual ~IRenderSurface() = default;

	virtual void bindRenderSurface( Graphics &gph ) cond_noex = 0;
	virtual void bindRenderSurface( Graphics &gph, IRenderSurface *pRs ) cond_noex = 0;
	virtual void clear( Graphics &gph, const std::array<float, 4> &color = {} ) cond_noex = 0;
};