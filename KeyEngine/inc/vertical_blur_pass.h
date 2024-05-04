#pragma once

#include "fullscreen_pass.h"
#include "constant_buffer_ex.h"


class Graphics;
class PixelShader;
class IRenderTargetView;

namespace ren
{

class VerticalBlurPass
	: public IFullscreenPass
{
	std::shared_ptr<PixelShaderConstantBufferEx> m_pPscbBlurDirection;
public:
	VerticalBlurPass( Graphics &gfx, const std::string &name );

	void run( Graphics &gfx ) const cond_noex override;
	void reset() cond_noex override;
};


}