#pragma once

#include "fullscreen_pass.h"
#include "texture.h"


namespace ren
{

class RttPassForPostProcessing
	: public IFullscreenPass
{
	std::shared_ptr<Texture> m_pOffscreenPostProcessTex;
public:
	RttPassForPostProcessing( Graphics &gph, const std::string &name, const unsigned rezReductFactor );

	void run( Graphics &gph ) const cond_noex override;
	void reset() cond_noex override;
};


}//ren