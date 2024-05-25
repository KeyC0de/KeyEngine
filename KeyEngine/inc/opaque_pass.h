#pragma once

#include "render_queue_pass.h"


class Graphics;
class Camera;

namespace ren
{

class OpaquePass
	: public RenderQueuePass
{
	const Camera *m_pActiveCamera = nullptr;
public:
	OpaquePass( Graphics &gfx, const std::string &name );

	void run( Graphics &gfx ) const cond_noex override;
	void setActiveCamera( const Camera &cam ) noexcept;
};


}