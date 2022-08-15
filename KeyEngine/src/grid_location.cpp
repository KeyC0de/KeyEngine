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

GridLocation& GridLocation::operator++()
{
	++x;
	++y;
	return *this;
}

GridLocation GridLocation::operator++() const
{
	GridLocation old = *this;
	++( *this );
	return old;
}

GridLocation& GridLocation::operator+=( const GridLocation &rhs )
{
	x = x + rhs.x;
	y = y + rhs.y;
	return *this;
}

GridLocation& GridLocation::operator--()
{
	--x;
	--y;
	return *this;
}

GridLocation GridLocation::operator--() const
{
	GridLocation old = *this;
	--( *this );
	return old;
}

GridLocation& GridLocation::operator-=( const GridLocation &rhs )
{
	x = x - rhs.x;
	y = y - rhs.y;
	return *this;
}

bool GridLocation::operator==( const GridLocation &rhs ) const
{
	return x == rhs.x && y == rhs.y;
}

bool GridLocation::operator!=( const GridLocation &rhs ) const
{
	return !( *this == rhs );
}


GridLocation operator+( GridLocation &lhs,
	const GridLocation &rhs )
{
	lhs += rhs;
	return lhs;
}

GridLocation operator-( GridLocation &lhs,
	const GridLocation &rhs )
{
	lhs -= rhs;
	return lhs;
}