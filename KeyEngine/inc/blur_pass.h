#pragma once

#include "fullscreen_pass.h"
#include "texture.h"


class Graphics;

namespace ren
{

//=============================================================
//	\class	BlurPass
//	\author	KeyC0de
//	\date	2022/10/03 10:31
//	\brief	A Post Processing Pass, reads texture and Blurs it
//=============================================================
class BlurPass
	: public IFullscreenPass
{
public:
	BlurPass( Graphics &gfx, const std::string &name );

	void reset() cond_noex override;
};


}