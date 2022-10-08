#pragma once

#include "render_queue_pass.h"


class Graphics;

namespace ren
{

//=============================================================
//	\class	BlurOutlineDrawPass
//	\author	KeyC0de
//	\date	2022/10/03 10:30
//	\brief	An offscreen Pass - Output only pass - RTV
//=============================================================
class BlurOutlineDrawPass
	: public RenderQueuePass
{
public:
	BlurOutlineDrawPass( Graphics &gph, const std::string &name, const unsigned rezReductFactor );

	void run( Graphics &gph ) const cond_noex override;
};


}