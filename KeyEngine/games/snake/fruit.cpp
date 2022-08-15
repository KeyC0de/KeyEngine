#include "fruit.h"
#include "graphics.h"
#include "snake.h"
#include "snake_play_field.h"


Fruit::Fruit( std::mt19937 &rng,
	const SnakePlayField &field,
	const SnakeRepr &snake )
{
	respawn( rng,
		field,
		snake );
}

void Fruit::render( Graphics &gph,
	SnakePlayField &field ) const
{
	field.renderCell( gph,
		m_gridLoc,
		m_color );
}

void Fruit::respawn( std::mt19937 &rng,
	const SnakePlayField &field,
	const SnakeRepr &snake )
{
	int min = 2 * field.m_cellSize;
	int xMax = field.getWidth() - field.m_cellSize;
	int yMax = field.getHeight() - field.m_cellSize;
	std::uniform_int_distribution<int> distrX{min, xMax};
	std::uniform_int_distribution<int> distrY{min, yMax};

	GridLocation newGridLoc;
	do
	{
		newGridLoc.x = distrX( rng ) / field.m_cellSize;
		newGridLoc.y = distrY( rng ) / field.m_cellSize;
	}
	while ( snake.checkForCollisions( newGridLoc ) );

	m_gridLoc = newGridLoc;
}

const GridLocation& Fruit::getGridLocation() const noexcept
{
	return m_gridLoc;
}