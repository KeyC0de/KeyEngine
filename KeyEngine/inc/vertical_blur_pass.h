#pragma once

#include "fullscreen_pass.h"
#include "constant_buffer_ex.h"


class Graphics;
class PixelShader;
class IRenderTargetView;

namespace ren
{

class VerticalBlurPass
	: public FullscreenPass
{
	std::shared_ptr<PixelConstantBufferEx> m_pPcbBlurDirection;
public:
	VerticalBlurPass( Graphics& gph, const std::string& name );

	void run( Graphics& gph ) const cond_noex override;
	void reset() cond_noex override;
};


}