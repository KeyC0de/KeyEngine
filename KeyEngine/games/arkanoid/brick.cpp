#include "brick.h"
#include "graphics.h"
#include "ball.h"
#include "assertions_console.h"


namespace dx = DirectX;

Brick::Brick( const Rect &rect,
	const ColorBGRA col )
	:
	m_rect{rect},
	m_color{col}
{

}

void Brick::render( Graphics &gph ) const
{
	if ( !m_bDestroyed )
	{
		gph.drawRect( m_rect.calcScaled( -m_margin ),
			m_color );
	}
}

bool Brick::checkForBallCollision( const Ball &ball ) const
{
	return !m_bDestroyed && m_rect.isOverlappingWith( ball.getRect() );
}

void Brick::doBallCollision( Ball &ball )
{
	ASSERT( checkForBallCollision( ball ), "There's no collision happening!" );

	const dx::XMFLOAT2 ballPos = ball.getPosition();
	if ( std::signbit( ball.getVelocity().x ) == std::signbit( ball.getPosition().x - calcCenter().x ) )
	{
		ball.reboundY();
	}
	else if ( ballPos.x >= m_rect.m_left && ballPos.x <= m_rect.m_right )
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