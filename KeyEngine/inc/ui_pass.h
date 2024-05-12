#pragma once

#include <memory>
#include <set>
#include "graphics_friend.h"
#include "DirectXTK/SpriteFont.h"
#include "DirectXTK/SpriteBatch.h"
#include "bindable_pass.h"
#include "ui_component.h"
#include "blend_state.h"
#include "texture.h"


namespace ren
{

class UIPass
	: public IBindablePass,
	public GraphicsFriend
{
	struct ComponentComparator
	{
		bool operator()( const std::unique_ptr<ui::Component> pComponent1, const std::unique_ptr<ui::Component> pComponent2 );
	};

	std::set<std::unique_ptr<ui::Component>> m_components;	// ui component hierarchy
	std::unique_ptr<DirectX::SpriteBatch> m_pSpriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_pFpsSpriteFont;
	std::shared_ptr<BlendState> m_pBlendState;
	std::shared_ptr<Texture> m_pTexture;
public:
	UIPass( Graphics &gfx, const std::string &name, const std::string &fpsFontNameNoExtension );

	virtual void run( Graphics &gfx ) const cond_noex override;
	virtual void reset() cond_noex override;
	void drawText( Graphics &gfx, const std::string &text, const DirectX::XMFLOAT2 &pos, const DirectX::XMVECTORF32 color = DirectX::Colors::White, const DirectX::XMFLOAT2 &scale = DirectX::XMFLOAT2{1.0f, 1.0f} );
	void drawTexture( Graphics &gfx, const int x, const int y, const int width, const int height );
private:
	//	\function	updateAndRenderFpsTimer	||	\date	2022/11/13 16:21
	//	\brief	draws FPS text
	void updateAndRenderFpsTimer( Graphics &gfx );
};


}//namespace ren}