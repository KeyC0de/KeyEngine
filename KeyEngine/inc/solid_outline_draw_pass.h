#pragma once

#include "render_queue_pass.h"


class Graphics;

namespace ren
{

class SolidOutlineDrawPass
	: public RenderQueuePass
{
public:
	SolidOutlineDrawPass( Graphics& gph, const std::string& name );
};


}//ren