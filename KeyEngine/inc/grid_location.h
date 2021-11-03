#pragma once


struct GridLocation
{
	int x;
	int y;

	GridLocation& operator++();
	GridLocation operator++() const;
	GridLocation& operator+=( const GridLocation& rhs );
	GridLocation operator+( const GridLocation& rhs );
	GridLocation& operator--();
	GridLocation operator--() const;
	GridLocation& operator-=( const GridLocation& rhs );
	GridLocation operator-( const GridLocation& rhs );
	bool operator==( const GridLocation& rhs ) const;
	bool operator!=( const GridLocation& rhs ) const;
};


struct Grid3dLocation final
	: public GridLocation
{
	int z;

	//TODO:
};