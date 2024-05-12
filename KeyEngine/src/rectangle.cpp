#include "rectangle.h"
#include <algorithm>


namespace dx = DirectX;

R3ctangle::R3ctangle( const float left,
	const float right,
	const float top,
	const float bottom )
	:
	m_left(left),
	m_right(right),
	m_top(top),
	m_bottom(bottom)
{

}

R3ctangle::R3ctangle( const dx::XMFLOAT2 &topLeft,
	const dx::XMFLOAT2 &bottomRight )
	:
	R3ctangle{topLeft.x, bottomRight.x, topLeft.y, bottomRight.y}
{

}

R3ctangle::R3ctangle( const dx::XMFLOAT2 &topLeft,
	const float width,
	const float height )
	:
	R3ctangle{topLeft.x, topLeft.x + width, topLeft.y, topLeft.y + height}
{

}

R3ctangle R3ctangle::makeGivenCenter( const dx::XMFLOAT2 &center,
	const float halfWidth,
	const float halfHeight)
{
	const dx::XMFLOAT2 topLeft{center.x - halfWidth, center.y - halfHeight};
	const dx::XMFLOAT2 bottomRight{center.x + halfWidth, center.y + halfHeight};
	return {topLeft, bottomRight};
}

bool R3ctangle::isOverlappingWith( const R3ctangle &other ) const noexcept
{
	/*return right > other.left
		&& left < other.right
		&& bottom > other.top
		&& top < other.bottom;*/
	return std::max( m_left, m_right ) > std::min( other.m_left, other.m_right )
		&& std::min( m_left, m_right ) < std::max( other.m_left, other.m_right )
		&& std::max( m_top, m_bottom ) > std::min( other.m_top, other.m_bottom )
		&& std::min( m_top, m_bottom ) < std::max( other.m_top, other.m_bottom );
}

R3ctangle R3ctangle::calcScaled( const float offset ) const noexcept
{
	return R3ctangle{m_left - offset, m_right + offset, m_top - offset, m_bottom + offset};
}

float R3ctangle::getWidth() const noexcept
{
	return m_right - m_left;
}

float R3ctangle::getHeight() const noexcept
{
	return m_bottom - m_top;
}

dx::XMFLOAT2 R3ctangle::calcCenter() const noexcept
{
	return dx::XMFLOAT2{(m_left + m_right) / 2.0f, (m_top + m_bottom) / 2.0f};
}