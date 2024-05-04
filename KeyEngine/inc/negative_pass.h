#pragma once

#include "fullscreen_pass.h"
#include "texture.h"


class Graphics;

namespace ren
{

//=============================================================
//	\class	BlurPass
//	\author	KeyC0de
//	\date	2022/10/29 23:54
//	\brief	A Post Processing Pass, reads texture and applies a digital negative effect
//=============================================================
class NegativePass
	: public IFullscreenPass
{
public:
	NegativePass( Graphics &gfx, const std::string &name );

	void reset() cond_noex override;
};


}//ren