#pragma once

#include "render_queue_pass.h"


class Graphics;

namespace ren
{

//=============================================================
//	\class	TransparentPass
//	\author	KeyC0de
//	\date	2024/05/05 14:42
//	\brief	A pass for transparent meshes
//=============================================================
class TransparentPass
	: public RenderQueuePass
{
public:
	TransparentPass( Graphics &gfx, const std::string &name );
};


}