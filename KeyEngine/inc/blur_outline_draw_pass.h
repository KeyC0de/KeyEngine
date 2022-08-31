#pragma once

#include "render_queue_pass.h"


class Graphics;

namespace ren
{

class BlurOutlineDrawPass
	: public RenderQueuePass
{
public:
	BlurOutlineDrawPass( Graphics &gph, const std::string &name, const int rezReductFactor );

	void run( Graphics &gph ) const cond_noex override;
};


}