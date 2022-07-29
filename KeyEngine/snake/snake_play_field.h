#pragma once

#include <DirectXMath.h>
#include <vector>
#include <random>
#include "color.h"


class Graphics;
struct GridLocation;
class SnakeRepr;

class SnakePlayField
{
	int m_xStart;
	int m_yStart;
	int m_width = 25;
	int m_height = 25;
public:
	static inline constexpr int m_cellSize = 10;

	enum class CellContents
	{
		Empty,
		Obstacle,
		Food,
		Poison
	};
private:
	std::vector<CellContents> contents;
public:
	SnakePlayField( Graphics &gph );
	SnakePlayField( Graphics &gph, int x, int y, int w, int h );

	void renderCell( Graphics &gph, const GridLocation &gridLoc, ColorBGRA col ) const;
	void renderBorders( Graphics &gph, ColorBGRA col ) const;
	void spawnContents( std::mt19937 & rng, const SnakeRepr &snake, CellContents contentsType );
	CellContents getContents( const GridLocation &loc ) const;
	bool isInside( const GridLocation &gridLoc ) const noexcept;
	int getStartX() const noexcept;
	int getStartY() const noexcept;
	int getWidth() const noexcept;
	int getHeight() const noexcept;
};