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

R3ctangle::R3ctangle( const dx::XMFLOAT2 &xAndY,
	const dx::XMFLOAT2 &bottomRight )
	:
	R3ctangle{xAndY.x, bottomRight.x, xAndY.y, bottomRight.y}
{

}

R3ctangle::R3ctangle( const dx::XMFLOAT2 &xAndY,
	const float width,
	const float height )
	:
	R3ctangle{xAndY.x, xAndY.x + width, xAndY.y, xAndY.y + height}
{

}

R3ctangle R3ctangle::makeGivenCenter( const dx::XMFLOAT2 &center,
	const float halfWidth,
	const float halfHeight)
{
	const dx::XMFLOAT2 xAndY{center.x - halfWidth, center.y - halfHeight};
	const dx::XMFLOAT2 bottomRight{center.x + halfWidth, center.y + halfHeight};
	return {xAndY, bottomRight};
}

bool R3ctangle::isOverlappingWith( const R3ctangle &other ) const noexcept
{
	return std::max( m_left, m_right ) > std::min( other.m_left, other.m_right )	// right > other.left
		&& std::min( m_left, m_right ) < std::max( other.m_left, other.m_right )	// && left < other.right
		&& std::max( m_top, m_bottom ) > std::min( other.m_top, other.m_bottom )	// && bottom > other.top
		&& std::min( m_top, m_bottom ) < std::max( other.m_top, other.m_bottom );	// && top < other.bottom;
}

R3ctangle R3ctangle::calcScaled( const float offset ) const noexcept
{
	return R3ctangle{m_left - offset, m_right + offset, m_top - offset, m_bottom + offset};
}

dx::XMFLOAT2 R3ctangle::calcCenter() const noexcept
{
	return dx::XMFLOAT2{(m_left + m_right) / 2.0f, (m_top + m_bottom) / 2.0f};
}

float R3ctangle::getLeft() const noexcept
{
	return m_left;
}

float& R3ctangle::getLeft()
{
	return m_left;
}

float R3ctangle::getX() const noexcept
{
	return m_left;
}

float& R3ctangle::getX()
{
	return m_left;
}

float R3ctangle::getRight() const noexcept
{
	return m_right;
}

float& R3ctangle::getRight()
{
	return m_right;
}

float R3ctangle::getTop() const noexcept
{
	return m_top;
}

float& R3ctangle::getTop()
{
	return m_top;
}

float R3ctangle::getY() const noexcept
{
	return m_top;
}

float& R3ctangle::getY()
{
	return m_top;
}

float R3ctangle::getBottom() const noexcept
{
	return m_bottom;
}

float& R3ctangle::getBottom()
{
	return m_bottom;
}

float R3ctangle::getWidth() const noexcept
{
	return m_right - m_left;
}

float R3ctangle::getHeight() const noexcept
{
	return m_bottom - m_top;
}
