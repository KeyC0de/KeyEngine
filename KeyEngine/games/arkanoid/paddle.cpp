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
	R3ctangle rect = this->rect();
	gfx.drawRectangle( rect, m_wingColor );
	rect.getLeft() += s_wingWidth;
	rect.getRight() -= s_wingWidth;
	gfx.drawRectangle( rect, m_color );
}

bool Paddle::doBallCollision( Ball &ball )
{
	if ( !m_bInCollisionCooldown )
	{
		const R3ctangle rect = this->rect();
		if ( rect.isOverlappingWith( ball.rect() ) )
		{
			const dx::XMFLOAT2 ballPos = ball.getPosition();
			if ( std::signbit( ball.getVelocity().x ) == std::signbit( ball.getPosition().x - m_posCenter.x ) )
			{// inside approach
				ball.reboundY();
			}
			else if ( ballPos.x >= rect.getLeft() && ballPos.x <= rect.getRight() )
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

void Paddle::doWallCollision( const R3ctangle &walls )
{
	const R3ctangle hitBox = rect();
	if ( hitBox.getLeft() < walls.getLeft() )
	{
		m_posCenter.x += walls.getLeft() - hitBox.getLeft();
	}
	else if ( hitBox.getRight() > walls.getRight() )
	{
		m_posCenter.x -= hitBox.getRight() - walls.getRight();
	}
}

R3ctangle Paddle::rect() const
{
	return R3ctangle::makeGivenCenter( m_posCenter, m_halfWidth, m_halfHeight );
}

void Paddle::resetCollisionCooldown()
{
	m_bInCollisionCooldown = false;
}

void Paddle::setPositionRel( const float val )
{
	m_posCenter.x += val;
}