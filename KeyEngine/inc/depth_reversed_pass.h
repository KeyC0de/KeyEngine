#pragma once

#include "render_queue_pass.h"


class Graphics;

namespace ren
{

class DepthReversedPass
	: public RenderQueuePass
{
public:
	DepthReversedPass( Graphics &gfx, const std::string &name );
};


}