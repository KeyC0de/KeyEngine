#pragma once

#include "bindable_pass.h"
#include "render_queue_pass.h"


namespace ren
{

class IFullscreenPass
	: public IBindablePass
{
	const char *m_fullscreenTag = "$fullscreen";
public:
	IFullscreenPass( Graphics &gph, const std::string &name ) cond_noex;

	void run( Graphics &gph ) const cond_noex override;
};

class FullscreenPass
	: public IFullscreenPass
{
public:
	FullscreenPass( Graphics &gph, const std::string &name );

	void run( Graphics &gph ) const cond_noex override;
	void reset() cond_noex override;
};


}//namespace ren