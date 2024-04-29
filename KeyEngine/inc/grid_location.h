#pragma once

#include <type_traits>


struct GridLocation
{
	int x;
	int y;

	friend GridLocation operator+( GridLocation &lhs, const GridLocation &rhs ) noexcept;
	friend GridLocation operator-( GridLocation &lhs, const GridLocation &rhs ) noexcept;

	GridLocation( const GridLocation &rhs );
	GridLocation& operator=( const GridLocation &rhs );

	GridLocation& operator++() noexcept;
	GridLocation operator++( int ) noexcept;
	GridLocation& operator+=( const GridLocation &rhs ) noexcept;
	GridLocation& operator--() noexcept;
	GridLocation operator--( int ) noexcept;
	GridLocation& operator-=( const GridLocation &rhs ) noexcept;
	bool operator==( const GridLocation &rhs ) const noexcept;
	bool operator!=( const GridLocation &rhs ) const noexcept;
};

GridLocation operator+( GridLocation &lhs, const GridLocation &rhs ) noexcept;
GridLocation operator-( GridLocation &lhs, const GridLocation &rhs ) noexcept;


struct Grid3dLocation final
	: public GridLocation
{
	int z;

	// #TODO
};