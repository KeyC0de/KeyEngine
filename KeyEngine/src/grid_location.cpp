#include "grid_location.h"


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

GridLocation& GridLocation::operator+=( const GridLocation& rhs )
{
	x = x + rhs.x;
	y = y + rhs.y;
	return *this;
}

GridLocation GridLocation::operator+( const GridLocation& rhs )
{
	( *this ) += rhs;
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

GridLocation& GridLocation::operator-=( const GridLocation& rhs )
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

GridLocation GridLocation::operator-( const GridLocation& rhs )
{
	( *this ) -= rhs;
	return *this;
}

bool GridLocation::operator==( const GridLocation& rhs ) const
{
	return x == rhs.x && y == rhs.y;
}

bool GridLocation::operator!=( const GridLocation& rhs ) const
{
	return !( *this == rhs );
}