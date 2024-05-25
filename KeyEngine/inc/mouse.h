#pragma once

#include <queue>
#include <optional>
#include "non_copyable.h"


class Mouse final
	: public NonCopyable
{
	friend class Window;

	static constexpr inline unsigned int s_maxBufferSize = 16u;

	struct RawDelta
	{
		int m_dx;
		int m_dy;
	};

	class Event final
	{
	public:
		enum Type
		{
			LMBPress,
			LMBRelease,
			RMBPress,
			RMBRelease,
			MMBPress,
			MMBRelease,
			WheelUp,
			WheelDown,
			Move,
			WindowEnter,
			WindowLeave,
			Invalid
		};

	private:
		Type m_type;
		bool m_bLmbPressed;
		bool m_bRmbPressed;
		bool m_bMmbPressed;
		int x;
		int y;
	public:
		Event( const Type type,
			const Mouse &mouse ) noexcept
			:
			m_type(type),
			m_bLmbPressed(mouse.m_bLmbPressed),
			m_bRmbPressed(mouse.m_bRmbPressed),
			m_bMmbPressed(mouse.m_bMmbPressed),
			x(mouse.m_x),
			y(mouse.m_y)
		{

		}

		constexpr bool isValid() const noexcept
		{
			return m_type != Invalid;
		}

		constexpr Type getType() const noexcept
		{
			return m_type;
		}

		constexpr std::pair<int, int> getPosition() const noexcept
		{
			return {x, y};
		}

		constexpr int getX() const noexcept
		{
			return x;
		}

		constexpr int getY() const noexcept
		{
			return y;
		}

		constexpr bool isLmbPressed() const noexcept
		{
			return m_bLmbPressed;
		}

		constexpr bool isLmbReleased() const noexcept
		{
			return m_bLmbPressed == false;
		}

		constexpr bool isRmbPressed() const noexcept
		{
			return m_bRmbPressed;
		}

		constexpr bool isRmbReleased() const noexcept
		{
			return m_bRmbPressed == false;
		}

		constexpr bool isMmbPressed() const noexcept
		{
			return m_bMmbPressed;
		}

		constexpr bool isMmbReleased() const noexcept
		{
			return m_bMmbPressed == false;
		}
	};

	int m_x = -1;
	int m_y = -1;
	bool m_bLmbPressed = false;
	bool m_bRmbPressed = false;
	bool m_bMmbPressed = false;
	bool m_bInWindow = false;
	int m_wheelDelta = 0;
	bool m_bRawEnabled = false;
	std::queue<Event> m_eventQueue;
	std::queue<RawDelta> m_rawDeltaBuffer;
private:
	Mouse() = default;
public:
	~Mouse() noexcept = default;
	Mouse( Mouse &&rhs ) noexcept;
	Mouse& operator=( Mouse &&rhs ) noexcept;

	std::pair<int, int> getPosition() const noexcept;
	int getX() const noexcept;
	int getY() const noexcept;
	bool isInWindow() const noexcept;
	bool isLmbPressed() const noexcept;
	bool isRmbPressed() const noexcept;
	bool isMmbPressed() const noexcept;
	std::optional<RawDelta> readRawDeltaBuffer() noexcept;
	std::optional<Event> readEventQueue() noexcept;
	bool isQueueEmpty() const noexcept;
	void flushEvents() noexcept;
	void enableRawInput() noexcept;
	void disableRawInput() noexcept;
	bool isRawInputEnabled() const noexcept;
private:
	void onMouseMove( const int newx, const int newy ) noexcept;
	void onMouseEnterWindow() noexcept;
	void onMouseLeaveWindow() noexcept;
	void onLmbPressed( const int x, const int y ) noexcept;
	void onLmbReleased( const int x, const int y ) noexcept;
	void onRmbPressed( const int x, const int y ) noexcept;
	void onRmbReleased( const int x, const int y ) noexcept;
	void onMmbPressed( const int x, const int y ) noexcept;
	void onMmbReleased( const int x, const int y ) noexcept;
	void onWheelDelta( const int x, const int y, const int delta ) noexcept;
	void onWheelUp( const int x, const int y ) noexcept;
	void onWheelDown( const int x, const int y ) noexcept;
	void onRawDelta( const int dx, const int dy ) noexcept;
	void trimEventQueue() noexcept;
	void trimRawInputBuffer() noexcept;
};