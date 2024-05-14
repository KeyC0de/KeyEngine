#include "brick.h"
#include "graphics.h"
#include "ball.h"
#include "assertions_console.h"


namespace dx = DirectX;

Brick::Brick( const R3ctangle &rect,
	const ColorBGRA col )
	:
	m_rect{rect},
	m_color{col}
{

}

void Brick::render( Graphics &gfx ) const cond_noex
{
	if ( !m_bDestroyed )
	{
		gfx.drawRectangle( m_rect.calcScaled( -s_margin ), m_color );
	}
}

bool Brick::checkForBallCollision( const Ball &ball ) const cond_noex
{
	return !m_bDestroyed && m_rect.isOverlappingWith( ball.rect() );
}

void Brick::doBallCollision( Ball &ball )
{
	ASSERT( checkForBallCollision( ball ), "There's no collision happening!" );

	const dx::XMFLOAT2 ballPos = ball.getPosition();
	if ( std::signbit( ball.getVelocity().x ) == std::signbit( ball.getPosition().x - calcCenter().x ) )
	{
		ball.reboundY();
	}
	else if ( ballPos.x >= m_rect.getLeft() && ballPos.x <= m_rect.getRight() )
	{
		ball.reboundY();
	}
	else
	{
		ball.reboundX();
	}
	m_bDestroyed = true;
}

const DirectX::XMFLOAT2 Brick::calcCenter() const noexcept
{
	return m_rect.calcCenter();
}