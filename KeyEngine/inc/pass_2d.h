#include "bindable_pass.h"


class Graphics;

namespace ren
{

class Pass2D
	: public IBindablePass
{

public:
	Pass2D( Graphics &gph, const std::string &name );

	void run( Graphics &gph ) const cond_noex override;
	void reset() cond_noex override;
};


}//ren