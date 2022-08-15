#pragma once

#include <DirectXMath.h>
#include "color.h"


class Graphics;
class Ball;
struct Rect;

class Paddle
{
	DirectX::XMFLOAT2 m_posCenter;
	static constexpr float m_wingWidth = 10.0f;
	ColorBGRA m_color;
	ColorBGRA m_wingColor;
	float m_halfWidth;
	float m_halfHeight;
	bool m_bCooldown = false;
public:
	Paddle( const DirectX::XMFLOAT2 &position, float width, float height, ColorBGRA col,
		ColorBGRA wingCol );

	void render( Graphics &gph ) const;
	bool doBallCollision( Ball &ball );
	void doWallCollision( const Rect &walls );
	Rect getRect() const;
	void resetCooldown();
	void setPositionRel( float val );
};