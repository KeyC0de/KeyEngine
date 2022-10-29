#pragma once

#include "fullscreen_pass.h"
#include "texture.h"


class Graphics;

namespace ren
{

//=============================================================
//	\class	PassThrough
//	\author	KeyC0de
//	\date	2022/10/29 23:57
//	\brief	A Post Processing Pass, reads offscreen texture and just copies it pixel per pixel to the back buffer
//=============================================================
class PassThrough
	: public IFullscreenPass
{
public:
	PassThrough( Graphics &gph, const std::string &name );

	void reset() cond_noex override;
};


}