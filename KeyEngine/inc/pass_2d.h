#include <array>
#include "bindable_pass.h"
#include "sprite.h"
#include "key_random.h"


class Graphics;

namespace ren
{

class Pass2D
	: public IBindablePass
{
	// All these should be in a Game class
	/*
	std::unique_ptr<Sprite> m_sprite;
	std::unique_ptr<SpriteSheet> m_player;
	std::array<int, 5> m_spriteOrder = {2, 3, 0, 6, 7};
	int m_currentSpriteIndex = 0;
	int g_width;
	int g_height;
	float y = 0.0f;
	float ySpeed = 1.0f;
	mutable KeyRandom m_randomness;
	*/
public:
	Pass2D( Graphics &gfx, const std::string &name );

	void run( Graphics &gfx ) const cond_noex override;
	void reset() cond_noex override;
};


}//namespace ren