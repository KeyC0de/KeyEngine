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
	const Camera *m_pMainCamera = nullptr;
public:
	LambertianPass( Graphics &gph, const std::string &name );
	
	void setMainCamera( const Camera &cam ) noexcept;
	void run( Graphics &gph ) const cond_noex override;
};


}