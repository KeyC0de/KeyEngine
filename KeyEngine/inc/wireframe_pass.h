#include "render_queue_pass.h"


class Graphics;

namespace ren
{

class WireframePass
	: public RenderQueuePass
{
public:
	WireframePass( Graphics &gfx, const std::string &name );
};


}//namespace ren