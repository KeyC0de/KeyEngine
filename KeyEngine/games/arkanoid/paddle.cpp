#include "paddle.h"
#include "ball.h"
#include "rectangle.h"
#include "graphics.h"


namespace dx = DirectX;

Paddle::Paddle( const dx::XMFLOAT2 &pos,
	float w,
	float h,
	ColorBGRA color,
	ColorBGRA wingColor )
	:
	m_posCenter{pos},
	m_halfWidth(w),
	m_halfHeight(h),
	m_color(color),
	m_wingColor(wingColor)
{

}

void Paddle::render( Graphics &gfx ) const cond_noex
{
	Rect rect = this->rect();
	gfx.drawRectangle( rect, m_wingColor );
	rect.m_left += s_wingWidth;
	rect.m_right -= s_wingWidth;
	gfx.drawRectangle( rect, m_color );
}

bool Paddle::doBallCollision( Ball &ball )
{
	if ( !m_bInCollisionCooldown )
	{
		const Rect rect = this->rect();
		if ( rect.isOverlappingWith( ball.rect() ) )
		{
			const dx::XMFLOAT2 ballPos = ball.getPosition();
			if ( std::signbit( ball.getVelocity().x ) == std::signbit( ball.getPosition().x - m_posCenter.x ) )
			{// inside approach
				ball.reboundY();
			}
			else if ( ballPos.x >= rect.m_left && ballPos.x <= rect.m_right )
			{// top-down collision
				ball.reboundY();
			}
			else
			{// side collision
				ball.reboundX();
			}
			m_bInCollisionCooldown = true;
			return true;
		}
	}
	return false;
}

void Paddle::doWallCollision( const Rect &walls )
{
	const Rect hitBox = rect();
	if ( hitBox.m_left < walls.m_left )
	{
		m_posCenter.x += walls.m_left - hitBox.m_left;
	}
	else if ( hitBox.m_right > walls.m_right )
	{
		m_posCenter.x -= hitBox.m_right - walls.m_right;
	}
}

Rect Paddle::rect() const
{
	return Rect::makeGivenCenter( m_posCenter, m_halfWidth, m_halfHeight );
}

void Paddle::resetCollisionCooldown()
{
	m_bInCollisionCooldown = false;
}

void Paddle::setPositionRel( const float val )
{
	m_posCenter.x += val;
}