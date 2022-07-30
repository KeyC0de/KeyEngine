#pragma once

#include <random>
#include "grid_location.h"
#include "color.h"


class Graphics;
class SnakeRepr;
class SnakePlayField;

class Fruit
{
	static inline ColorBGRA m_color = col::Red;
	GridLocation m_gridLoc;
public:
	Fruit( std::mt19937 &rng, const SnakePlayField &field, const SnakeRepr &snake );

	void render( Graphics &gph, SnakePlayField &field ) const;
	void respawn( std::mt19937 &rng, const SnakePlayField &field, const SnakeRepr &snake );
	const GridLocation& getGridLocation() const noexcept;
};