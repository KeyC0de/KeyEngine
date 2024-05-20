#pragma once

#include <memory>
#include "graphics_friend.h"


namespace DirectX
{

class SpriteBatch;
class SpriteFont;

}

class RasterizerState;
class Texture;

namespace gui
{

class Component;

class UIPass
	: public GraphicsFriend
{
	std::unique_ptr<Component> m_pRoot;
	std::unique_ptr<DirectX::SpriteBatch> m_pSpriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_pSpriteFont;
	std::shared_ptr<RasterizerState> m_pRasterizerState;
public:
	UIPass( Graphics &gfx );

	~UIPass() noexcept;

	void recreate( Graphics &gfx );
	void update( const float dt, const std::pair<int, int> inputXandY, const float lerpBetweenFrames ) cond_noex;
	void render( Graphics &gfx ) const cond_noex;
	const std::unique_ptr<Component>& getRoot() const noexcept;
	std::unique_ptr<Component>& getRoot();
};


}//namespace gui