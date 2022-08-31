#include "snake_play_field.h"
#include "grid_location.h"
#include "graphics.h"
#include "snake.h"
#include "grid_location.h"
#include "assertions_console.h"


namespace dx = DirectX;

SnakePlayField::SnakePlayField( Graphics &gph )
{
	m_xStart = m_cellSize;
	m_yStart = m_cellSize;
}

SnakePlayField::SnakePlayField( Graphics &gph,
	const int x,
	const int y,
	const int w,
	const int h )
	:
	m_xStart(x),
	m_yStart(y),
	m_width(w),
	m_height(h)
{

}

void SnakePlayField::renderCell( Graphics &gph,
	const GridLocation &gridLoc,
	const ColorBGRA col ) const
{
	ASSERT( gridLoc.x >= m_xStart, "!( gridLoc.x >= xStart )" );
	ASSERT( gridLoc.x < m_width, "!( gridLoc.x < m_width )" );
	ASSERT( gridLoc.y >= m_yStart, "!( gridLoc.y >= yStart )" );
	ASSERT( gridLoc.y < m_height, "!( gridLoc.x < m_height )" );
	gph.drawRectWH( gridLoc.x * m_cellSize,
		gridLoc.y * m_cellSize,
		m_cellSize,
		m_cellSize,
		col );
}

void SnakePlayField::renderBorders( Graphics &gph,
	const ColorBGRA col ) const
{
	gph.drawRectangle( m_xStart, m_yStart, m_width - m_xStart, m_cellSize, col );	// top border
	gph.drawRectangle( m_xStart, m_height, m_width - m_xStart, m_cellSize, col );	// bottom border
	gph.drawRectangle( m_yStart, m_yStart, m_cellSize, m_height - m_yStart, col );	// left border
	gph.drawRectangle( m_width, m_yStart, m_cellSize, m_height - m_yStart, col );	// right border
}

void SnakePlayField::spawnContents( std::mt19937 & rng,
	const SnakeRepr &snake,
	const CellContents contentsType )
{
	std::uniform_int_distribution<int> xDist( 0, m_width - 1 );
	std::uniform_int_distribution<int> yDist( 0, m_height - 1 );

	GridLocation newLoc;
	do
	{
		newLoc.x = xDist( rng );
		newLoc.y = yDist( rng );
	}
	while( snake.isInside( newLoc ) || getContents( newLoc ) != CellContents::Empty );

	contents[newLoc.y * m_width + newLoc.x] = contentsType;
}

SnakePlayField::CellContents SnakePlayField::getContents( const GridLocation &loc ) const
{
	return contents[loc.y * m_width + loc.x];
}

bool SnakePlayField::isInside( const GridLocation &gridLoc ) const noexcept
{
	int nx = gridLoc.x * m_cellSize;
	int ny = gridLoc.y * m_cellSize;
	return nx >= m_xStart + m_cellSize && nx <= m_width - m_cellSize
		&& ny >= m_yStart + m_cellSize && ny <= m_height - m_cellSize;
}

const int SnakePlayField::getStartX() const noexcept
{
	return m_xStart;
}

const int SnakePlayField::getStartY() const noexcept
{
	return m_yStart;
}

const int SnakePlayField::getWidth() const noexcept
{
	return m_width;
}

const int SnakePlayField::getHeight() const noexcept
{
	return m_height;
}