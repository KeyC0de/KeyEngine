#pragma once

#include <memory>
#include "graphics_friend.h"
#include "SpriteFont.h"
#include "SpriteBatch.h"
#include "key_timer.h"
#include "bindable_pass.h"


namespace ren
{

// #TODO: extend this into UiPass ...
class FontPass
	: public IBindablePass,
	public GraphicsFriend
{
	std::unique_ptr<DirectX::SpriteBatch> m_pSpriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_pFpsSpriteFont;
	KeyTimer<std::chrono::microseconds> m_fpsTimer;
public:
	FontPass( Graphics &gph, const std::string &name, const std::string &fpsFontNameNoExtension );

	virtual void run( Graphics &gph ) const cond_noex override;
	virtual void reset() cond_noex override;
private:
	//	\function	updateAndRenderFpsTimer	||	\date	2022/11/13 16:21
	//	\brief	draws FPS text
	void updateAndRenderFpsTimer();
};


}//namespace ren}