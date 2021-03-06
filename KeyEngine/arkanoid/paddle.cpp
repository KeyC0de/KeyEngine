#include "paddle.h"
#include "ball.h"
#include "rectangle.h"
#include "graphics.h"


namespace dx = DirectX;

Paddle::Paddle( const dx::XMFLOAT2 &pos,
	float w,
	float h,
	ColorBGRA col,
	ColorBGRA wingCol )
	:
	m_posCenter{pos},
	m_halfWidth(w),
	m_halfHeight(h),
	m_color(col),
	m_wingColor(wingCol)
{

}

void Paddle::render( Graphics &gph ) const
{
	Rect rect = getRect();
	gph.drawRect( rect,
		m_wingColor );
	rect.m_left += m_wingWidth;
	rect.m_right -= m_wingWidth;
	gph.drawRect( rect,
		m_color );
}

bool Paddle::doBallCollision( Ball &ball )
{
	if ( !m_bCooldown )
	{
		const Rect rect = getRect();
		if ( rect.isOverlappingWith( ball.getRect() ) )
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
			m_bCooldown = true;
			return true;
		}
	}
	return false;
}

void Paddle::doWallCollision( const Rect &walls )
{
	const Rect hitBox = getRect();
	if ( hitBox.m_left < walls.m_left )
	{
		m_posCenter.x += walls.m_left - hitBox.m_left;
	}
	else if ( hitBox.m_right > walls.m_right )
	{
		m_posCenter.x -= hitBox.m_right - walls.m_right;
	}
}

Rect Paddle::getRect() const
{
	return Rect::makeGivenCenter( m_posCenter,
		m_halfWidth,
		m_halfHeight );
}

void Paddle::resetCooldown()
{
	m_bCooldown = false;
}

void Paddle::setPositionRel( float val )
{
	m_posCenter.x += val;
}