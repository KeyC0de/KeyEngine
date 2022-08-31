#pragma once


namespace gph_mode
{

enum Mode
{
	_2D,
	_3D,
};

static inline constexpr const int current_mode = _3D;

static inline constexpr const int get() noexcept
{
	return current_mode;
}


};