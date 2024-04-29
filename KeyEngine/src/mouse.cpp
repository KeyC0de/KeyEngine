#include "winner.h"
#include "mouse.h"
#include <utility>


Mouse::Mouse( Mouse &&rhs ) noexcept
	:
	m_x{std::move( rhs.m_x )},
	m_y{std::move( rhs.m_y )},
	m_bLmbPressed{std::move( rhs.m_bLmbPressed )},
	m_bRmbPressed{std::move( rhs.m_bRmbPressed )},
	m_bMmbPressed{std::move( rhs.m_bMmbPressed )},
	m_bInWindow{std::move( rhs.m_bInWindow )},
	m_wheelDelta{std::move( rhs.m_wheelDelta )},
	m_bRawEnabled{std::move( rhs.m_bRawEnabled )},
	m_eventQueue{std::move( rhs.m_eventQueue )},
	m_rawDeltaBuffer{std::move( rhs.m_rawDeltaBuffer )}
{
	rhs.m_eventQueue = {};
	rhs.m_rawDeltaBuffer = {};
}

Mouse& Mouse::operator=( Mouse &&rhs ) noexcept
{
	Mouse tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

std::pair<int, int> Mouse::getPosition() const noexcept
{
	return {m_x, m_y};
}

int Mouse::getX() const noexcept
{
	return m_x;
}

int Mouse::getY() const noexcept
{
	return m_y;
}

bool Mouse::isInWindow() const noexcept
{
	return m_bInWindow;
}

bool Mouse::isLmbPressed() const noexcept
{
	return m_bLmbPressed;
}

bool Mouse::isRmbPressed() const noexcept
{
	return m_bRmbPressed;
}

bool Mouse::isMmbPressed() const noexcept
{
	return m_bMmbPressed;
}

std::optional<Mouse::RawDelta> Mouse::readRawDeltaBuffer() noexcept
{
	if ( m_rawDeltaBuffer.empty() )
	{
		return std::nullopt;
	}
	const RawDelta d = m_rawDeltaBuffer.front();
	m_rawDeltaBuffer.pop();
	return d;
}

std::optional<Mouse::Event> Mouse::readEventQueue() noexcept
{
	if ( !m_eventQueue.empty() )
	{
		Mouse::Event e = m_eventQueue.front();
		m_eventQueue.pop();
		return e;
	}
	return std::nullopt;
}

bool Mouse::isQueueEmpty() const noexcept
{
	return m_eventQueue.empty();
}

void Mouse::flushEvents() noexcept
{
	m_eventQueue = std::queue<Event>{};
}

void Mouse::enableRawInput() noexcept
{
	m_bRawEnabled = true;
}

void Mouse::disableRawInput() noexcept
{
	m_bRawEnabled = false;
}

bool Mouse::isRawInputEnabled() const noexcept
{
	return m_bRawEnabled;
}

void Mouse::onMouseMove( int newx,
	int newy ) noexcept
{
	m_x = newx;
	m_y = newy;
	m_eventQueue.push( Event{Event::Type::Move, *this} );
	trimEventQueue();
}

void Mouse::onMouseEnterWindow() noexcept
{
	m_bInWindow = true;
	m_eventQueue.push( Event{Event::Type::WindowEnter, *this} );
	trimEventQueue();
}

void Mouse::onMouseLeaveWindow() noexcept
{
	m_bInWindow = false;
	m_eventQueue.push( Event{Event::Type::WindowLeave, *this} );
	trimEventQueue();
}

void Mouse::onLmbPressed( const int x,
	const int y ) noexcept
{
	m_bLmbPressed = true;
	m_eventQueue.push( Event{Event::Type::LMBPress, *this} );
	trimEventQueue();
}

void Mouse::onLmbReleased( const int x,
	const int y ) noexcept
{
	m_bLmbPressed = false;
	m_eventQueue.push( Event{Event::Type::LMBRelease, *this} );
	trimEventQueue();
}

void Mouse::onRmbPressed( const int x,
	const int y ) noexcept
{
	m_bRmbPressed = true;
	m_eventQueue.push( Event{Event::Type::RMBPress, *this} );
	trimEventQueue();
}

void Mouse::onRmbReleased( const int x,
	const int y ) noexcept
{
	m_bRmbPressed = false;
	m_eventQueue.push( Event{Event::Type::RMBRelease, *this} );
	trimEventQueue();
}

void Mouse::onMmbPressed( const int x,
	const int y ) noexcept
{
	m_bMmbPressed = true;
	m_eventQueue.push( Event{Event::Type::MMBPress, *this} );
	trimEventQueue();
}

void Mouse::onMmbReleased( const int x,
	const int y ) noexcept
{
	m_bMmbPressed = false;
	m_eventQueue.push( Event{Event::Type::MMBRelease, *this} );
	trimEventQueue();
}

void Mouse::onWheelDelta( const int x,
	const int y,
	const int delta ) noexcept
{
	m_wheelDelta += delta;
	// generate events per mouse wheel notch - 120 units
	while ( m_wheelDelta >= WHEEL_DELTA )
	{
		m_wheelDelta -= WHEEL_DELTA;
		onWheelUp( x, y );
	}
	while ( m_wheelDelta <= -WHEEL_DELTA )
	{
		m_wheelDelta += WHEEL_DELTA;
		onWheelDown( x, y );
	}
}

void Mouse::onWheelUp( const int x,
	const int y ) noexcept
{
	m_eventQueue.push( Event{Event::Type::WheelUp, *this} );
	trimEventQueue();
}

void Mouse::onWheelDown( const int x,
	const int y ) noexcept
{
	m_eventQueue.push( Event{Event::Type::WheelDown, *this} );
	trimEventQueue();
}

void Mouse::onRawDelta( const int dx,
	const int dy ) noexcept
{
	m_rawDeltaBuffer.push( {dx, dy} );
	trimRawInputBuffer();
}

void Mouse::trimEventQueue() noexcept
{
	while ( m_eventQueue.size() > s_maxBufferSize )
	{
		m_eventQueue.pop();
	}
}

void Mouse::trimRawInputBuffer() noexcept
{
	while ( m_rawDeltaBuffer.size() > s_maxBufferSize )
	{
		m_rawDeltaBuffer.pop();
	}
}