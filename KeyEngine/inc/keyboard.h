#pragma once

#include <queue>
#include <bitset>
#include <optional>
#include "non_copyable.h"


class Keyboard final
	: NonCopyable
{
	friend class Window;
public:
	static constexpr inline unsigned int s_nKeys = 256u;
	static constexpr inline unsigned int s_maxBufferSize = 16u;

	class Event final
	{
	public:
		enum Type
		{
			Press,
			Release,
			Invalid
		};
	private:
		Type m_type;
		unsigned char m_keyc0de;
	public:
		Event( const Type type,
			unsigned char keyc0de ) noexcept
			:
			m_type(type),
			m_keyc0de(keyc0de)
		{

		}

		constexpr bool isPressEvent() const noexcept
		{
			return m_type == Press;
		}

		constexpr bool isReleaseEvent() const noexcept
		{
			return m_type == Release;
		}

		constexpr unsigned char getKeycode() const noexcept
		{
			return m_keyc0de;
		}
	};
	bool m_bAutorepeat = false;
	std::bitset<s_nKeys> m_keyStates;
	std::queue<Keyboard::Event> m_eventQueue;
	std::queue<char> m_charBuffer;
private:
	Keyboard() = default;

	//===================================================
	//	\function	trimBuffer
	//	\brief  check whether the provided buffer needs trimming
	//	\date	2022/08/26 23:19
	template<typename T>
	static void trimBuffer( std::queue<T>& buf ) noexcept;
public:
	~Keyboard() noexcept = default;
	Keyboard( Keyboard &&rhs ) noexcept;
	Keyboard& operator=( Keyboard &&rhs ) noexcept;

	// key events
	bool isKeyPressed( unsigned char keycode ) const noexcept;
	//===================================================
	//	\function	readEventQueue
	//	\brief  read event from the queue if any
	//	\date	2022/08/26 23:19
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