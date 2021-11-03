#pragma once

#include "render_queue_pass.h"


class Graphics;

namespace ren
{

class SolidOutlineMaskPass
	: public RenderQueuePass
{
public:
	SolidOutlineMaskPass( Graphics& gph, const std::string& name );
};


}//ren