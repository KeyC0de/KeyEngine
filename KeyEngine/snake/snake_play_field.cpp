#include "snake_play_field.h"
#include "grid_location.h"
#include "graphics.h"
#include "snake.h"
#include "grid_location.h"
#include "assertions_console.h"


namespace dx = DirectX;

SnakePlayField::SnakePlayField( Graphics& gph )
{
	m_xStart = m_cellSize;
	m_yStart = m_cellSize;
}

SnakePlayField::SnakePlayField( Graphics& gph,
	int x,
	int y,
	int w,
	int h )
	:
	m_xStart(x),
	m_yStart(y),
	m_width(w),
	m_height(h)
{

}

void SnakePlayField::renderCell( Graphics& gph,
	const GridLocation& gridLoc,
	ColorBGRA col ) const
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

void SnakePlayField::renderBorders( Graphics& gph,
	ColorBGRA col ) const
{
	gph.drawRect( m_xStart, m_yStart, m_width - m_xStart, m_cellSize, col );	// top border
	gph.drawRect( m_xStart, m_height, m_width - m_xStart, m_cellSize, col );	// bottom border
	gph.drawRect( m_yStart, m_yStart, m_cellSize, m_height - m_yStart, col );	// left border
	gph.drawRect( m_width, m_yStart, m_cellSize, m_height - m_yStart, col );	// right border
}

void SnakePlayField::spawnContents( std::mt19937 & rng,const SnakeRepr & snake,CellContents contentsType )
{
	std::uniform_int_distribution<int> xDist( 0, m_width - 1 );
	std::uniform_int_distribution<int> yDist( 0, m_height - 1 );

	GridLocation newLoc;
	do
	{
		newLoc.x = xDist( rng );
		newLoc.y = yDist( rng );
	}
	while( snake.IsInTile( newLoc ) || getContents( newLoc ) != CellContents::Empty );

	contents[newLoc.y * width + newLoc.x] = contentsType;
}

SnakePlayField::CellContents SnakePlayField::getContents( const GridLocation& loc ) const
{
	return contents[loc.y * width + loc.x];
}

bool SnakePlayField::isInside( const GridLocation& gridLoc ) const noexcept
{
	int nx = gridLoc.x * m_cellSize;
	int ny = gridLoc.y * m_cellSize;
	return nx >= m_xStart + m_cellSize && nx <= m_width - m_cellSize
		&& ny >= m_yStart + m_cellSize && ny <= m_height - m_cellSize;
}

int SnakePlayField::getStartX() const noexcept
{
	return m_xStart;
}

int SnakePlayField::getStartY() const noexcept
{
	return m_yStart;
}

int SnakePlayField::getWidth() const noexcept
{
	return m_width;
}

int SnakePlayField::getHeight() const noexcept
{
	return m_height;
}