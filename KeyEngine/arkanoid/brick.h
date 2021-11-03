#pragma once

#include <DirectXMath.h>
#include "rectangle.h"
#include "color.h"


class Graphics;
class Ball;

class Brick
{
	static constexpr float m_margin = 1.0f;
	Rect m_rect;
	ColorBGRA m_color;
	bool m_bDestroyed = false;
public:
	Brick() = default;
	Brick( const Rect& rect, const ColorBGRA col );

	void render( Graphics& gph ) const;
	bool checkForBallCollision( const Ball& ball ) const;
	// performs the collision
	void doBallCollision( Ball& ball );
	DirectX::XMFLOAT2 getCenter() const noexcept;
};