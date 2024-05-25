#pragma once

#include "render_queue_pass.h"


class Graphics;
class Camera;

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
	const Camera *m_pActiveCamera = nullptr;
public:
	TransparentPass( Graphics &gfx, const std::string &name );

	void run( Graphics &gfx ) const cond_noex override;
	void setActiveCamera( const Camera &cam ) noexcept;
};


}