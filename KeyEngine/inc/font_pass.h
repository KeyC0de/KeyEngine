#pragma once

#include <memory>
#include "graphics_friend.h"
#include "DirectXTK/SpriteFont.h"
#include "DirectXTK/SpriteBatch.h"
#include "bindable_pass.h"


namespace ren
{

class FontPass
	: public IBindablePass,
	public GraphicsFriend
{
	std::unique_ptr<DirectX::SpriteBatch> m_pSpriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_pFpsSpriteFont;
public:
	FontPass( Graphics &gfx, const std::string &name, const std::string &fpsFontNameNoExtension );

	virtual void run( Graphics &gfx ) const cond_noex override;
	virtual void reset() cond_noex override;
private:
	//	\function	updateAndRenderFpsTimer	||	\date	2022/11/13 16:21
	//	\brief	draws FPS text
	void updateAndRenderFpsTimer( Graphics &gfx );
};


}//namespace ren}