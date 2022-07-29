#pragma once

#include <queue>
#include <bitset>
#include <optional>


class Keyboard final
{
	friend class Window;

public:
	class Event final
	{
	public:
		enum class Type
		{
			Press,
			Release,
			Invalid
		};
	private:
		Type m_type;
		unsigned char m_keyc0de;
	public:
		Event( Type type,
			unsigned char keyc0de ) noexcept
			:
			m_type(type),
			m_keyc0de(keyc0de)
		{

		}
		
		constexpr bool isPressEvent() const noexcept
		{
			return m_type == Type::Press;
		}
		
		constexpr bool isReleaseEvent() const noexcept
		{
			return m_type == Type::Release;
		}
		
		constexpr unsigned char getKeycode() const noexcept
		{
			return m_keyc0de;
		}
	};
	static constexpr inline unsigned int m_nKeys = 256u;
	static constexpr inline unsigned int m_maxBufferSize = 16u;
	bool m_bAutorepeat = false;
	std::bitset<m_nKeys> m_keyStates;
	std::queue<Keyboard::Event> m_eventQueue;
	std::queue<char> m_charBuffer;
private:
	Keyboard() = default;

	// check whether the provided buffer needs trimming
	template<typename T>
	static void trimBuffer( std::queue<T>& buf ) noexcept;
public:
	~Keyboard() noexcept = default;
	Keyboard( const Keyboard &rhs ) = delete;
	Keyboard &operator=( const Keyboard &rhs ) = delete;
	Keyboard( Keyboard&& rhs ) noexcept;
	Keyboard &operator=( Keyboard&& rhs ) noexcept;

	// key events
	bool isKeyPressed( unsigned char keycode ) const noexcept;
	// read event from the queue if any
	std::optional<Keyboard::Event> readEventQueue() noexcept;
	bool isEventQueueEmpty() const noexcept;
	void flushEventQueue() noexcept;
	// char events
	std::optional<char> readCharBuffer() noexcept;
	bool isCharBufferEmpty() const noexcept;
	void flushCharBuffer() noexcept;
	void flushAll() noexcept;
	// autorepeat control
	void enableAutorepeat() noexcept;
	void disableAutorepeat() noexcept;
	bool isAutorepeatEnabled() const noexcept;
private:
	void onKeyPressed( unsigned char keycode ) noexcept;
	void onKeyReleased( unsigned char keycode ) noexcept;
	void onChar( char c ) noexcept;
	void clearKeyStates() noexcept;
};