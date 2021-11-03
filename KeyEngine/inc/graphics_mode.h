#pragma once


class GraphicsMode final
{
public:
	enum Mode
	{
		_2D = 0,
		_3D = 1
	};
private:
	static constexpr inline int m_current = _2D;
public:
	static constexpr int get() noexcept
	{
		return m_current;
	}
};