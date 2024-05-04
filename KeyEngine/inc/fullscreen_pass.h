#pragma once

#include "bindable_pass.h"
#include "render_queue_pass.h"


namespace ren
{

class IFullscreenPass
	: public IBindablePass
{
	static constexpr const char *s_fullscreenTag = "$fullscreen";
public:
	IFullscreenPass( Graphics &gfx, const std::string &name ) cond_noex;

	void run( Graphics &gfx ) const cond_noex override;
};


}//namespace ren