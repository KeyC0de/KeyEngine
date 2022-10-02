#pragma once

#include "fullscreen_pass.h"


class Graphics;

namespace ren
{

class BlurPass
	: public FullscreenPass
{
public:
	BlurPass( Graphics &gph, const std::string &name, const int rezReductFactor );

	void run( Graphics &gph ) const cond_noex override;
	void reset() cond_noex override;
};


}