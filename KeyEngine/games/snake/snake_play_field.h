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
	SnakePlayField( Graphics &gph, const int x, const int y, const int w, const int h );

	void renderCell( Graphics &gph, const GridLocation &gridLoc, const ColorBGRA col ) const cond_noex;
	void renderBorders( Graphics &gph, const ColorBGRA col ) const cond_noex;
	void spawnContents( std::mt19937 &rng, const SnakeRepr &snake, CellContents contentsType );
	CellContents getContents( const GridLocation &loc ) const;
	bool isInside( const GridLocation &gridLoc ) const noexcept;
	const int getStartX() const noexcept;
	const int getStartY() const noexcept;
	const int getWidth() const noexcept;
	const int getHeight() const noexcept;
};