#include "grid_location.h"


GridLocation::GridLocation( const GridLocation &rhs )
	:
	x{rhs.x},
	y{rhs.y}
{

}

GridLocation& GridLocation::operator=( const GridLocation &rhs )
{
	GridLocation temp{rhs};
	std::swap( this->x, temp.x );
	std::swap( this->y, temp.y );
	return *this;
}

GridLocation& GridLocation::operator++() noexcept
{
	++x;
	++y;
	return *this;
}

GridLocation GridLocation::operator++( int ) noexcept
{
	GridLocation old{*this};
	operator++();
	return old;
}

GridLocation& GridLocation::operator+=( const GridLocation &rhs ) noexcept
{
	x = x + rhs.x;
	y = y + rhs.y;
	return *this;
}

GridLocation& GridLocation::operator--() noexcept
{
	--x;
	--y;
	return *this;
}

GridLocation GridLocation::operator--( int ) noexcept
{
	GridLocation old{*this};
	operator--();
	return old;
}

GridLocation& GridLocation::operator-=( const GridLocation &rhs ) noexcept
{
	x = x - rhs.x;
	y = y - rhs.y;
	return *this;
}

bool GridLocation::operator==( const GridLocation &rhs ) const noexcept
{
	return x == rhs.x && y == rhs.y;
}

bool GridLocation::operator!=( const GridLocation &rhs ) const noexcept
{
	return !( *this == rhs );
}


GridLocation operator+( GridLocation &lhs,
	const GridLocation &rhs ) noexcept
{
	lhs += rhs;
	return lhs;
}

GridLocation operator-( GridLocation &lhs,
	const GridLocation &rhs ) noexcept
{
	lhs -= rhs;
	return lhs;
}