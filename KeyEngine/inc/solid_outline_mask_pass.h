#pragma once

#include "render_queue_pass.h"


class Graphics;

namespace ren
{

//=============================================================
//	\class	SolidOutlineMaskPass
//	\author	KeyC0de
//	\date	2022/04/27 20:33
//	\brief	requires objects to use VertexShader flat_vs.cso Bindable
//=============================================================
class SolidOutlineMaskPass
	: public RenderQueuePass
{
public:
	SolidOutlineMaskPass( Graphics &gph, const std::string &name );
};


}//ren