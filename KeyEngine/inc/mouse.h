#pragma once

#include <queue>
#include <optional>


class Mouse final
{
	friend class Window;

	struct RawDelta
	{
		int m_dx;
		int m_dy;
	};
	
	class Event final
	{
	public:
		enum class Type
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
		Event( Type type,
			const Mouse &mouse ) noexcept
			:
			m_type(type),
			m_bLmbPressed(mouse.m_bLmbPressed),
			m_bRmbPressed(mouse.m_bRmbPressed),
			x(mouse.m_x),
			y(mouse.m_y)
		{

		}

		constexpr bool isValid() const noexcept
		{
			return m_type != Type::Invalid;
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

		constexpr bool isRmbPressed() const noexcept
		{
			return m_bRmbPressed;
		}

		constexpr bool isMmbPressed() const noexcept
		{
			return m_bMmbPressed;
		}
	};

	static constexpr inline unsigned int m_maxBufferSize = 16u;
	int m_x;
	int m_y;
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
	Mouse( const Mouse &rhs ) = delete;
	Mouse &operator=( const Mouse &rhs ) = delete;
	Mouse( Mouse&& rhs ) noexcept;
	Mouse &operator=( Mouse&& rhs ) noexcept;

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
	void onMouseMove( int newx, int newy ) noexcept;
	void onMouseEnterWindow() noexcept;
	void onMouseLeaveWindow() noexcept;
	void onLmbPressed( int x, int y ) noexcept;
	void onLmbReleased( int x, int y ) noexcept;
	void onRmbPressed( int x, int y ) noexcept;
	void onRmbReleased( int x, int y ) noexcept;
	void onMmbPressed( int x, int y ) noexcept;
	void onMmbReleased( int x, int y ) noexcept;
	void onWheelDelta( int x, int y, int delta ) noexcept;
	void onWheelUp( int x, int y ) noexcept;
	void onWheelDown( int x, int y ) noexcept;
	void onRawDelta( int dx, int dy ) noexcept;
	void trimEventQueue() noexcept;
	void trimRawInputBuffer() noexcept;
};