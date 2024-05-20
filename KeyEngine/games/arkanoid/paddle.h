#pragma once

#include <DirectXMath.h>
#include "color.h"


class Graphics;
class Ball;
class RectangleF;

class Paddle
{
	DirectX::XMFLOAT2 m_posCenter;
	static constexpr float s_wingWidth = 10.0f;
	ColorBGRA m_color;
	ColorBGRA m_wingColor;
	float m_halfWidth;
	float m_halfHeight;
	bool m_bInCollisionCooldown = false;
public:
	Paddle( const DirectX::XMFLOAT2 &position, const float width, const float height, const ColorBGRA color, const ColorBGRA wingColor );

	void render( Graphics &gfx ) const cond_noex;
	bool doBallCollision( Ball &ball );
	void doWallCollision( const RectangleF &walls );
	RectangleF rect() const;
	void resetCollisionCooldown();
	void setPositionRel( const float val );
};