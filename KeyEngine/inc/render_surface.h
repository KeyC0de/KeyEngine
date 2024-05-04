#pragma once

#include <array>


class Graphics;

class IRenderSurface
{
public:
	virtual ~IRenderSurface() = default;

	virtual void bindRenderSurface( Graphics &gfx ) cond_noex = 0;
	virtual void bindRenderSurface( Graphics &gfx, IRenderSurface *pRs ) cond_noex = 0;
	virtual void clear( Graphics &gfx, const std::array<float, 4> &color = {} ) cond_noex = 0;
};