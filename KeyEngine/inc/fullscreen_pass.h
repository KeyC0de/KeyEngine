#pragma once

#include "bindable_pass.h"


namespace ren
{

class FullscreenPass
	: public IBindablePass
{
public:
	FullscreenPass( Graphics &gph, const std::string &name ) cond_noex;

	// Child::run must call FullscreenPass:run as a last step
	void run( Graphics &gph ) const cond_noex override;
};


}