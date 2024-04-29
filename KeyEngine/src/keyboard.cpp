#include "keyboard.h"


Keyboard::Keyboard( Keyboard &&rhs ) noexcept
	:
	m_bAutorepeat{rhs.m_bAutorepeat},
	m_keyStates{std::move( rhs.m_keyStates )},
	m_eventQueue{std::move( rhs.m_eventQueue )},
	m_charBuffer{std::move( rhs.m_charBuffer )}
{
	rhs.m_keyStates = {};
	rhs.m_eventQueue = {};
	rhs.m_charBuffer = {};
}

Keyboard& Keyboard::operator=( Keyboard &&rhs ) noexcept
{
	Keyboard tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

bool Keyboard::isKeyPressed( unsigned char keycode ) const noexcept
{
	return m_keyStates[keycode];
}

std::optional<Keyboard::Event> Keyboard::readEventQueue() noexcept
{
	if ( !m_eventQueue.empty() )
	{
		Event ev = m_eventQueue.front();
		m_eventQueue.pop();
		return ev;
	}
	return std::nullopt;
}

bool Keyboard::isEventQueueEmpty() const noexcept
{
	return m_eventQueue.empty();
}

void Keyboard::flushEventQueue() noexcept
{
	m_eventQueue = std::queue<Event>{};
}

std::optional<char> Keyboard::readCharBuffer() noexcept
{
	if ( !m_charBuffer.empty() )
	{
		unsigned char charcode = m_charBuffer.front();
		m_charBuffer.pop();
		return charcode;
	}
	return std::nullopt;
}

bool Keyboard::isCharBufferEmpty() const noexcept
{
	return m_charBuffer.empty();
}

void Keyboard::flushCharBuffer() noexcept
{
	m_charBuffer = std::queue<char>{};
}

void Keyboard::flushAll() noexcept
{
	flushEventQueue();
	flushCharBuffer();
}

void Keyboard::enableAutorepeat() noexcept
{
	m_bAutorepeat = true;
}

void Keyboard::disableAutorepeat() noexcept
{
	m_bAutorepeat = false;
}

bool Keyboard::isAutorepeatEnabled() const noexcept
{
	return m_bAutorepeat;
}

void Keyboard::onKeyPressed( unsigned char keycode ) noexcept
{
	m_keyStates[keycode] = true;
	m_eventQueue.push( Event{Event::Type::Press, keycode} );
	trimBuffer( m_eventQueue );
}

void Keyboard::onKeyReleased( unsigned char keycode ) noexcept
{
	m_keyStates[keycode] = false;
	m_eventQueue.push( Event{Event::Type::Release, keycode} );
	trimBuffer( m_eventQueue );
}

void Keyboard::onChar( char c ) noexcept
{
	m_charBuffer.push( c );
	trimBuffer( m_charBuffer );
}

void Keyboard::clearKeyStates() noexcept
{
	m_keyStates.reset();
}

template<typename T>
void Keyboard::trimBuffer( std::queue<T>& buf ) noexcept
{
	while ( buf.size() > s_maxBufferSize )
	{
		buf.pop();
	}
}