#pragma once

#include "bindable_pass.h"


namespace ren
{

class FullscreenPass
	: public IBindablePass
{
	const char *m_fullscreenTag = "$fullscreen";
public:
	FullscreenPass( Graphics &gph, const std::string &name ) cond_noex;

	//===================================================
	//	\function	run
	//	\brief  Child::run must call FullscreenPass:run as a last step
	//	\date	2022/10/02 10:48
	void run( Graphics &gph ) const cond_noex override;
};


}