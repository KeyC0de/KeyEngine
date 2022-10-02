#pragma once

#include "fullscreen_pass.h"


class Graphics;

namespace ren
{

class BlurPass
	: public FullscreenPass
{
public:
	BlurPass( Graphics &gph, const std::string &name );

	void reset() cond_noex override;
};


}