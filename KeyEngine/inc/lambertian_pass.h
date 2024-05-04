#pragma once

#include <memory>
#include "render_queue_pass.h"


class Graphics;
class Camera;

namespace ren
{

class LambertianPass
	: public RenderQueuePass
{
	const Camera *m_pActiveCamera = nullptr;
public:
	LambertianPass( Graphics &gfx, const std::string &name );

	void setActiveCamera( const Camera &cam ) noexcept;
	void run( Graphics &gfx ) const cond_noex override;
};


}