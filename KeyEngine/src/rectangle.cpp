#include "rectangle.h"
#include <algorithm>


namespace dx = DirectX;

#pragma warning( disable : 4244 )
RectangleF::RectangleF( const int x,
	const int y,
	const int width,
	const int height )
	:
	m_left(x),
	m_right(x + width),
	m_top(y),
	m_bottom(y + height)
{

}
#pragma warning( default : 4244 )

RectangleF::RectangleF( const float left,
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

RectangleF::RectangleF( const dx::XMFLOAT2 &xAndY,
	const dx::XMFLOAT2 &bottomRight )
	:
	RectangleF{xAndY.x, bottomRight.x, xAndY.y, bottomRight.y}
{

}

RectangleF::RectangleF( const dx::XMFLOAT2 &xAndY,
	const float width,
	const float height )
	:
	RectangleF{xAndY.x, xAndY.x + width, xAndY.y, xAndY.y + height}
{

}

RectangleF RectangleF::makeGivenCenter( const dx::XMFLOAT2 &center,
	const float halfWidth,
	const float halfHeight)
{
	const dx::XMFLOAT2 xAndY{center.x - halfWidth, center.y - halfHeight};
	const dx::XMFLOAT2 bottomRight{center.x + halfWidth, center.y + halfHeight};
	return {xAndY, bottomRight};
}

bool RectangleF::isOverlappingWith( const RectangleF &other ) const noexcept
{
	return std::max( m_left, m_right ) > std::min( other.m_left, other.m_right )	// right > other.left
		&& std::min( m_left, m_right ) < std::max( other.m_left, other.m_right )	// && left < other.right
		&& std::max( m_top, m_bottom ) > std::min( other.m_top, other.m_bottom )	// && bottom > other.top
		&& std::min( m_top, m_bottom ) < std::max( other.m_top, other.m_bottom );	// && top < other.bottom;
}

RectangleF RectangleF::calcScaled( const float offset ) const noexcept
{
	return RectangleF{m_left - offset, m_right + offset, m_top - offset, m_bottom + offset};
}

dx::XMFLOAT2 RectangleF::calcCenter() const noexcept
{
	return dx::XMFLOAT2{(m_left + m_right) / 2.0f, (m_top + m_bottom) / 2.0f};
}

float RectangleF::getLeft() const noexcept
{
	return m_left;
}

float& RectangleF::getLeft()
{
	return m_left;
}

float RectangleF::getX() const noexcept
{
	return m_left;
}

float& RectangleF::getX()
{
	return m_left;
}

float RectangleF::getRight() const noexcept
{
	return m_right;
}

float& RectangleF::getRight()
{
	return m_right;
}

float RectangleF::getTop() const noexcept
{
	return m_top;
}

float& RectangleF::getTop()
{
	return m_top;
}

float RectangleF::getY() const noexcept
{
	return m_top;
}

float& RectangleF::getY()
{
	return m_top;
}

float RectangleF::getBottom() const noexcept
{
	return m_bottom;
}

float& RectangleF::getBottom()
{
	return m_bottom;
}

float RectangleF::getWidth() const noexcept
{
	return m_right - m_left;
}

float RectangleF::getHeight() const noexcept
{
	return m_bottom - m_top;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
RectangleI::RectangleI( const int x,
	const int y,
	const int width,
	const int height )
	:
	m_left(x),
	m_right(x + width),
	m_top(y),
	m_bottom(y + height)
{

}

RectangleI RectangleI::makeGivenCenter( const dx::XMFLOAT2 &center,
	const int halfWidth,
	const int halfHeight )
{
	const dx::XMFLOAT2 xAndY{center.x - halfWidth, center.y - halfHeight};
	const dx::XMFLOAT2 bottomRight{center.x + halfWidth, center.y + halfHeight};
	const int x = static_cast<int>( xAndY.x );
	const int y = static_cast<int>( xAndY.y );
	const int width = static_cast<int>( bottomRight.x ) - x;
	const int height = static_cast<int>( bottomRight.y ) - y;
	return {x, y, width, height};
}

bool RectangleI::isOverlappingWith( const RectangleI &other ) const noexcept
{
	return std::max( m_left, m_right ) > std::min( other.m_left, other.m_right )	// right > other.left
		&& std::min( m_left, m_right ) < std::max( other.m_left, other.m_right )	// && left < other.right
		&& std::max( m_top, m_bottom ) > std::min( other.m_top, other.m_bottom )	// && bottom > other.top
		&& std::min( m_top, m_bottom ) < std::max( other.m_top, other.m_bottom );	// && top < other.bottom;
}

RectangleI RectangleI::calcScaled( const int offset ) const noexcept
{
	return RectangleI{m_left - offset, m_right + offset, m_top - offset, m_bottom + offset};
}

dx::XMFLOAT2 RectangleI::calcCenter() const noexcept
{
	return dx::XMFLOAT2{(m_left + m_right) / 2.0f, (m_top + m_bottom) / 2.0f};
}

int RectangleI::getLeft() const noexcept
{
	return m_left;
}

int& RectangleI::getLeft()
{
	return m_left;
}

int RectangleI::getX() const noexcept
{
	return m_left;
}

int& RectangleI::getX()
{
	return m_left;
}

int RectangleI::getRight() const noexcept
{
	return m_right;
}

int& RectangleI::getRight()
{
	return m_right;
}

int RectangleI::getTop() const noexcept
{
	return m_top;
}

int& RectangleI::getTop()
{
	return m_top;
}

int RectangleI::getY() const noexcept
{
	return m_top;
}

int& RectangleI::getY()
{
	return m_top;
}

int RectangleI::getBottom() const noexcept
{
	return m_bottom;
}

int& RectangleI::getBottom()
{
	return m_bottom;
}

int RectangleI::getWidth() const noexcept
{
	return m_right - m_left;
}

int RectangleI::getHeight() const noexcept
{
	return m_bottom - m_top;
}