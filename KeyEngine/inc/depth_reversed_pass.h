#pragma once

#include "render_queue_pass.h"


class Graphics;

namespace ren
{

class DepthReversedPass
	: public RenderQueuePass
{
public:
	DepthReversedPass( Graphics &gph, const std::string &name );
};


}