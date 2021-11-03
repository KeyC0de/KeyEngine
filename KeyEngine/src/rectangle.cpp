#include "rectangle.h"

#ifndef maxNum
#define maxNum( a, b ) ( ( (a) > (b) ) ? (a) : (b) )
#endif

#ifndef minNum
#define minNum( a, b ) ( ( (a) < (b) ) ? (a) : (b) )
#endif


namespace dx = DirectX;

Rect::Rect( float left,
	float right,
	float top,
	float bottom )
	:
	m_left(left),
	m_right(right),
	m_top(top),
	m_bottom(bottom)
{

}

Rect::Rect( const dx::XMFLOAT2& topLeft,
	const dx::XMFLOAT2& bottomRight )
	:
	Rect{topLeft.x, bottomRight.x, topLeft.y, bottomRight.y}
{

}

Rect::Rect( const dx::XMFLOAT2& topLeft,
	float width,
	float height )
	:
	Rect{topLeft.x, topLeft.x + width, topLeft.y, topLeft.y + height}
{

}

Rect Rect::makeGivenCenter( const dx::XMFLOAT2& center,
	float halfWidth,
	float halfHeight)
{
	const dx::XMFLOAT2 topLeft{center.x - halfWidth, center.y - halfHeight};
	const dx::XMFLOAT2 bottomRight{center.x + halfWidth, center.y + halfHeight};
	return {topLeft, bottomRight};
}

bool Rect::isOverlappingWith( const Rect& other ) const noexcept
{
	/*return right > other.left
		&& left < other.right
		&& bottom > other.top
		&& top < other.bottom;*/
	return maxNum( m_left, m_right ) > minNum( other.m_left, other.m_right )
		&& minNum( m_left, m_right ) < maxNum( other.m_left, other.m_right )
		&& maxNum( m_top, m_bottom ) > minNum( other.m_top, other.m_bottom )
		&& minNum( m_top, m_bottom ) < maxNum( other.m_top, other.m_bottom );
}

Rect Rect::getScaled( float offset ) const noexcept
{
	return Rect{m_left - offset, m_right + offset, m_top - offset, m_bottom + offset};
}

float Rect::getRectWidth() const noexcept
{
	return m_right - m_left;
}

float Rect::getRectHeight() const noexcept
{
	return m_bottom - m_top;
}

dx::XMFLOAT2 Rect::getCenter() const noexcept
{
	return dx::XMFLOAT2{(m_left + m_right) / 2.0f, (m_top + m_bottom) / 2.0f};
}