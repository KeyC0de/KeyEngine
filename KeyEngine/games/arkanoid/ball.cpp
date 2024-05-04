#include "ball.h"
#include "rectangle.h"
#include "graphics.h"


namespace dx = DirectX;

Ball::Ball( const dx::XMFLOAT2 &position,
	const dx::XMFLOAT2 &velocity )
	:
	m_pos{position},
	m_vel{velocity}
{

}

void Ball::update( const float dt )
{
	dx::XMFLOAT2 change{m_vel.x * dt, m_vel.y * dt};
	dx::XMStoreFloat2( &m_pos, {m_pos.x + change.x, m_pos.y + change.y} );
}

bool Ball::doWallCollision( const Rect &wall )
{
	bool collided = false;
	const Rect rect = this->rect();
	if ( rect.m_left < wall.m_left )
	{
		m_pos.x += wall.m_left - rect.m_left;
		reboundX();
		collided = true;
	}
	else if ( rect.m_right > wall.m_right )
	{
		m_pos.x -= rect.m_right - wall.m_right;
		reboundX();
		collided = true;
	}

	if ( rect.m_top < wall.m_top )
	{
		m_pos.y += wall.m_top - rect.m_top;
		reboundY();
		collided = true;
	}
	else if ( rect.m_bottom > wall.m_bottom )
	{
		m_pos.y -= rect.m_bottom - wall.m_bottom;
		reboundY();
		collided = true;
	}
	return collided;
}

void Ball::reboundX()
{
	m_vel.x = -m_vel.x;
}

void Ball::reboundY()
{
	m_vel.y = -m_vel.y;
}

Rect Ball::rect() const noexcept
{
	return Rect::makeGivenCenter( m_pos, s_radius, s_radius );
}

const dx::XMFLOAT2& Ball::getVelocity() const noexcept
{
	return m_vel;
}

const dx::XMFLOAT2& Ball::getPosition() const noexcept
{
	return m_pos;
}

void Ball::render( Graphics &gfx ) cond_noex
{
	const int x = (int) m_pos.x - (int) s_radius;
	const int y = (int) m_pos.y - (int) s_radius;

#pragma region ballPixels
	gfx.putPixel( 5 + x, 0 + y, col::Silver );
	gfx.putPixel( 6 + x, 0 + y, col::Silver );
	gfx.putPixel( 7 + x, 0 + y, col::Silver );
	gfx.putPixel( 8 + x, 0 + y, col::Silver );
	gfx.putPixel( 3 + x, 1 + y, col::Silver );
	gfx.putPixel( 4 + x, 1 + y, col::Silver );
	gfx.putPixel( 5 + x, 1 + y, col::Silver );
	gfx.putPixel( 6 + x, 1 + y, col::Silver );
	gfx.putPixel( 7 + x, 1 + y, col::Silver );
	gfx.putPixel( 8 + x, 1 + y, col::Silver );
	gfx.putPixel( 9 + x, 1 + y, col::Silver );
	gfx.putPixel( 10 + x, 1 + y, col::Silver );
	gfx.putPixel( 2 + x, 2 + y, col::Silver );
	gfx.putPixel( 3 + x, 2 + y, col::Silver );
	gfx.putPixel( 4 + x, 2 + y, col::Silver );
	gfx.putPixel( 5 + x, 2 + y, col::Silver );
	gfx.putPixel( 6 + x, 2 + y, col::Silver );
	gfx.putPixel( 7 + x, 2 + y, col::Silver );
	gfx.putPixel( 8 + x, 2 + y, col::Silver );
	gfx.putPixel( 9 + x, 2 + y, col::Silver );
	gfx.putPixel( 10 + x, 2 + y, col::Silver );
	gfx.putPixel( 11 + x, 2 + y, col::Silver );
	gfx.putPixel( 1 + x, 3 + y, col::Silver );
	gfx.putPixel( 2 + x, 3 + y, col::Silver );
	gfx.putPixel( 3 + x, 3 + y, col::Silver );
	gfx.putPixel( 4 + x, 3 + y, col::Silver );
	gfx.putPixel( 5 + x, 3 + y, col::Silver );
	gfx.putPixel( 6 + x, 3 + y, col::Silver );
	gfx.putPixel( 7 + x, 3 + y, col::Silver );
	gfx.putPixel( 8 + x, 3 + y, col::Silver );
	gfx.putPixel( 9 + x, 3 + y, col::Silver );
	gfx.putPixel( 10 + x, 3 + y, col::Silver );
	gfx.putPixel( 11 + x, 3 + y, col::Silver );
	gfx.putPixel( 12 + x, 3 + y, col::Silver );
	gfx.putPixel( 1 + x, 4 + y, col::Silver );
	gfx.putPixel( 2 + x, 4 + y, col::Silver );
	gfx.putPixel( 3 + x, 4 + y, col::Silver );
	gfx.putPixel( 4 + x, 4 + y, col::Silver );
	gfx.putPixel( 5 + x, 4 + y, col::Silver );
	gfx.putPixel( 6 + x, 4 + y, col::Silver );
	gfx.putPixel( 7 + x, 4 + y, col::Silver );
	gfx.putPixel( 8 + x, 4 + y, col::Silver );
	gfx.putPixel( 9 + x, 4 + y, col::Silver );
	gfx.putPixel( 10 + x, 4 + y, col::Silver );
	gfx.putPixel( 11 + x, 4 + y, col::Silver );
	gfx.putPixel( 12 + x, 4 + y, col::Silver );
	gfx.putPixel( 0 + x, 5 + y, col::Silver );
	gfx.putPixel( 1 + x, 5 + y, col::Silver );
	gfx.putPixel( 2 + x, 5 + y, col::Silver );
	gfx.putPixel( 3 + x, 5 + y, col::Silver );
	gfx.putPixel( 4 + x, 5 + y, col::Silver );
	gfx.putPixel( 5 + x, 5 + y, col::Silver );
	gfx.putPixel( 6 + x, 5 + y, col::Silver );
	gfx.putPixel( 7 + x, 5 + y, col::Silver );
	gfx.putPixel( 8 + x, 5 + y, col::Silver );
	gfx.putPixel( 9 + x, 5 + y, col::Silver );
	gfx.putPixel( 10 + x, 5 + y, col::Silver );
	gfx.putPixel( 11 + x, 5 + y, col::Silver );
	gfx.putPixel( 12 + x, 5 + y, col::Silver );
	gfx.putPixel( 13 + x, 5 + y, col::Silver );
	gfx.putPixel( 0 + x, 6 + y, col::Silver );
	gfx.putPixel( 1 + x, 6 + y, col::Silver );
	gfx.putPixel( 2 + x, 6 + y, col::Silver );
	gfx.putPixel( 3 + x, 6 + y, col::Silver );
	gfx.putPixel( 4 + x, 6 + y, col::Silver );
	gfx.putPixel( 5 + x, 6 + y, col::Silver );
	gfx.putPixel( 6 + x, 6 + y, col::Silver );
	gfx.putPixel( 7 + x, 6 + y, col::Silver );
	gfx.putPixel( 8 + x, 6 + y, col::Silver );
	gfx.putPixel( 9 + x, 6 + y, col::Silver );
	gfx.putPixel( 10 + x, 6 + y, col::Silver );
	gfx.putPixel( 11 + x, 6 + y, col::Silver );
	gfx.putPixel( 12 + x, 6 + y, col::Silver );
	gfx.putPixel( 13 + x, 6 + y, col::Silver );
	gfx.putPixel( 0 + x, 7 + y, col::Silver );
	gfx.putPixel( 1 + x, 7 + y, col::Silver );
	gfx.putPixel( 2 + x, 7 + y, col::Silver );
	gfx.putPixel( 3 + x, 7 + y, col::Silver );
	gfx.putPixel( 4 + x, 7 + y, col::Silver );
	gfx.putPixel( 5 + x, 7 + y, col::Silver );
	gfx.putPixel( 6 + x, 7 + y, col::Silver );
	gfx.putPixel( 7 + x, 7 + y, col::Silver );
	gfx.putPixel( 8 + x, 7 + y, col::Silver );
	gfx.putPixel( 9 + x, 7 + y, col::Silver );
	gfx.putPixel( 10 + x, 7 + y, col::Silver );
	gfx.putPixel( 11 + x, 7 + y, col::Silver );
	gfx.putPixel( 12 + x, 7 + y, col::Silver );
	gfx.putPixel( 13 + x, 7 + y, col::Silver );
	gfx.putPixel( 0 + x, 8 + y, col::Silver );
	gfx.putPixel( 1 + x, 8 + y, col::Silver );
	gfx.putPixel( 2 + x, 8 + y, col::Silver );
	gfx.putPixel( 3 + x, 8 + y, col::Silver );
	gfx.putPixel( 4 + x, 8 + y, col::Silver );
	gfx.putPixel( 5 + x, 8 + y, col::Silver );
	gfx.putPixel( 6 + x, 8 + y, col::Silver );
	gfx.putPixel( 7 + x, 8 + y, col::Silver );
	gfx.putPixel( 8 + x, 8 + y, col::Silver );
	gfx.putPixel( 9 + x, 8 + y, col::Silver );
	gfx.putPixel( 10 + x, 8 + y, col::Silver );
	gfx.putPixel( 11 + x, 8 + y, col::Silver );
	gfx.putPixel( 12 + x, 8 + y, col::Silver );
	gfx.putPixel( 13 + x, 8 + y, col::Silver );
	gfx.putPixel( 1 + x, 9 + y, col::Silver );
	gfx.putPixel( 2 + x, 9 + y, col::Silver );
	gfx.putPixel( 3 + x, 9 + y, col::Silver );
	gfx.putPixel( 4 + x, 9 + y, col::Silver );
	gfx.putPixel( 5 + x, 9 + y, col::Silver );
	gfx.putPixel( 6 + x, 9 + y, col::Silver );
	gfx.putPixel( 7 + x, 9 + y, col::Silver );
	gfx.putPixel( 8 + x, 9 + y, col::Silver );
	gfx.putPixel( 9 + x, 9 + y, col::Silver );
	gfx.putPixel( 10 + x, 9 + y, col::Silver );
	gfx.putPixel( 11 + x, 9 + y, col::Silver );
	gfx.putPixel( 12 + x, 9 + y, col::Silver );
	gfx.putPixel( 1 + x, 10 + y, col::Silver );
	gfx.putPixel( 2 + x, 10 + y, col::Silver );
	gfx.putPixel( 3 + x, 10 + y, col::Silver );
	gfx.putPixel( 4 + x, 10 + y, col::Silver );
	gfx.putPixel( 5 + x, 10 + y, col::Silver );
	gfx.putPixel( 6 + x, 10 + y, col::Silver );
	gfx.putPixel( 7 + x, 10 + y, col::Silver );
	gfx.putPixel( 8 + x, 10 + y, col::Silver );
	gfx.putPixel( 9 + x, 10 + y, col::Silver );
	gfx.putPixel( 10 + x, 10 + y, col::Silver );
	gfx.putPixel( 11 + x, 10 + y, col::Silver );
	gfx.putPixel( 12 + x, 10 + y, col::Silver );
	gfx.putPixel( 2 + x, 11 + y, col::Silver );
	gfx.putPixel( 3 + x, 11 + y, col::Silver );
	gfx.putPixel( 4 + x, 11 + y, col::Silver );
	gfx.putPixel( 5 + x, 11 + y, col::Silver );
	gfx.putPixel( 6 + x, 11 + y, col::Silver );
	gfx.putPixel( 7 + x, 11 + y, col::Silver );
	gfx.putPixel( 8 + x, 11 + y, col::Silver );
	gfx.putPixel( 9 + x, 11 + y, col::Silver );
	gfx.putPixel( 10 + x, 11 + y, col::Silver );
	gfx.putPixel( 11 + x, 11 + y, col::Silver );
	gfx.putPixel( 3 + x, 12 + y, col::Silver );
	gfx.putPixel( 4 + x, 12 + y, col::Silver );
	gfx.putPixel( 5 + x, 12 + y, col::Silver );
	gfx.putPixel( 6 + x, 12 + y, col::Silver );
	gfx.putPixel( 7 + x, 12 + y, col::Silver );
	gfx.putPixel( 8 + x, 12 + y, col::Silver );
	gfx.putPixel( 9 + x, 12 + y, col::Silver );
	gfx.putPixel( 10 + x, 12 + y, col::Silver );
	gfx.putPixel( 5 + x, 13 + y, col::Silver );
	gfx.putPixel( 6 + x, 13 + y, col::Silver );
	gfx.putPixel( 7 + x, 13 + y, col::Silver );
	gfx.putPixel( 8 + x, 13 + y, col::Silver );
#pragma endregion ballPixels
}