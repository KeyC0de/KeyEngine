#pragma once

#include "fullscreen_pass.h"


class Graphics;

namespace ren
{

//=============================================================
//	\class	BlurPass
//	\author	KeyC0de
//	\date	2022/10/03 10:31
//	\brief	An offscreen Pass - Output only - RTV
//=============================================================
class BlurPass
	: public IFullscreenPass
{
public:
	BlurPass( Graphics &gph, const std::string &name, const unsigned rezReductFactor );

	//void run( Graphics &gph ) const cond_noex override;
	void reset() cond_noex override;
};


}