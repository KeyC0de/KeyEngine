#pragma once

#include "fullscreen_pass.h"
#include "texture.h"


class Graphics;

namespace ren
{

///=============================================================
/// \class	PassThrough
/// \author	KeyC0de
/// \date	2022/10/29 23:57
/// \brief	A Post Processing Pass, reads texture and simply outputs its values
///=============================================================
class PassThrough
	: public IFullscreenPass
{
public:
	PassThrough( Graphics &gfx, const std::string &name );

	void reset() cond_noex override;
};


}