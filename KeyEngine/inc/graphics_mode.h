#pragma once


namespace gph_mode
{

enum Mode
{
	_2D,
	_3D,
};

static inline constexpr const int s_currentMode = _3D;	/// you have to manually change this to switch modes (and potentially games)

static inline constexpr const int get() noexcept
{
	return s_currentMode;
}

};