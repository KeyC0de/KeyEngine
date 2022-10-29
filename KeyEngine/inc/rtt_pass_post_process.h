#pragma once

#include "fullscreen_pass.h"
#include "texture.h"


namespace ren
{

class BasePostProcessingPass
	: public IFullscreenPass
{
	//std::shared_ptr<IRenderTargetView> m_pRtv;
	std::shared_ptr<TextureOffscreenRT> m_pOffscreenPostProcessTex;
public:
	BasePostProcessingPass( Graphics &gph, const std::string &name, const unsigned rezReductFactor );

	void run( Graphics &gph ) const cond_noex override;
	void reset() cond_noex override;
};


}//ren