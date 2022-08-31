#pragma once

#include <type_traits>


struct GridLocation
{
	int x;
	int y;

	friend GridLocation operator+( GridLocation &lhs, const GridLocation &rhs );
	friend GridLocation operator-( GridLocation &lhs, const GridLocation &rhs );

	GridLocation( const GridLocation &rhs );
	GridLocation& operator=( const GridLocation &rhs );

	GridLocation& operator++();
	GridLocation operator++() const;
	GridLocation& operator+=( const GridLocation &rhs );
	GridLocation& operator--();
	GridLocation operator--() const;
	GridLocation& operator-=( const GridLocation &rhs );

	bool operator==( const GridLocation &rhs ) const;
	bool operator!=( const GridLocation &rhs ) const;
};

GridLocation operator+( GridLocation &lhs, const GridLocation &rhs );
GridLocation operator-( GridLocation &lhs, const GridLocation &rhs );


struct Grid3dLocation final
	: public GridLocation
{
	int z;

	// #TODO
};