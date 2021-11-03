#pragma once

#include "render_queue_pass.h"


class Graphics;

namespace ren
{

class BlurOutlineMaskPass
	: public RenderQueuePass
{
public:
	BlurOutlineMaskPass( Graphics& gph, const std::string& name );
};


}