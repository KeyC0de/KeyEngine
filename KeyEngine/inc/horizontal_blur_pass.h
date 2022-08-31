#pragma once

#include "fullscreen_pass.h"
#include "constant_buffer_ex.h"


class Graphics;
class PixelShader;
class IRenderTargetView;

namespace ren
{

class HorizontalBlurPass
	: public FullscreenPass
{
	std::shared_ptr<PixelShaderConstantBufferEx> m_pPscbBlurDirection;
public:
	HorizontalBlurPass( Graphics &gph, const std::string &name, const int rezReductFactor );

	void run( Graphics &gph ) const cond_noex override;
	void reset() cond_noex override;
};


}