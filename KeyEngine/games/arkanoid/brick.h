#pragma once

#include <DirectXMath.h>
#include "rectangle.h"
#include "color.h"


class Graphics;
class Ball;

class Brick
{
	static constexpr float s_margin = 1.0f;
	Rect m_rect;
	ColorBGRA m_color;
	bool m_bDestroyed = false;
public:
#pragma warning( disable : 26495 )
	Brick() = default;
#pragma warning( default : 26495 )
	Brick( const Rect &rect, const ColorBGRA col );

	void render( Graphics &gph ) const cond_noex;
	bool checkForBallCollision( const Ball &ball ) const cond_noex;
	void doBallCollision( Ball &ball );
	const DirectX::XMFLOAT2 calcCenter() const noexcept;
};