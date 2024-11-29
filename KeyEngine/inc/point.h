#pragma once

#include <utility>


namespace gui
{

struct Point
{
	int x;
	int y;

	Point( const int x,
		const int y )
		:
		x(x),
		y(y)
	{}

	operator std::pair<const int, const int>() const noexcept
	{
		return {x, y};
	}

	operator std::pair<int, int>() const noexcept
	{
		return {x, y};
	}

	operator std::pair<int, int>()
	{
		return {x, y};
	}
};

static const Point g_point_zero{0, 0};

}//namespace gui