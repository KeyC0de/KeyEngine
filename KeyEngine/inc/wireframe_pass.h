#include "render_queue_pass.h"


class Graphics;

namespace ren
{

class WireframePass
	: public RenderQueuePass
{
public:
	WireframePass( Graphics &gph, const std::string &name );
};


}//ren