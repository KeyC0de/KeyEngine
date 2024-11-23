#pragma once

#include <thread>
#include <chrono>
#include "console.h"
#include "non_copyable.h"
#include "math_utils.h"


///=============================================================
/// \class	KeyTimer<Resolution>
/// \author	KeyC0de
/// \date	2020/09/23 15:26
/// \brief	the KeyTimer that records time like a badass
/// \brief	Resolution specified the accuracy of the KeyTimer
/// \brief	output values are always displayed in milliseconds
/// \brief	the KeyTimer can also wait for a specified amount of time pausing execution
///=============================================================
template<typename Resolution = std::chrono::milliseconds>
class KeyTimer final
	: NonCopyableAndNonMovable
{
public:
	using TClock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
		std::chrono::high_resolution_clock,
		std::chrono::steady_clock>;
private:
	TClock::time_point m_start{};	// records starting time
	TClock::time_point m_last{};	// counts duration between successive calls of getTime
	size_t m_duration{};			// counts duration from starting time
public:
	explicit KeyTimer( bool bStart = false ) noexcept
	{
		if ( bStart )
		{
			start();
		}
	}

	~KeyTimer() noexcept
	{
		size_t dur = getMilliSecondsFromStart();
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( std::to_string( dur ) + "\n"s );
#endif
	}

	void start() noexcept
	{
		m_start = TClock::now();
		m_last = TClock::now();
		m_duration = 0;
	}

	void restart() noexcept
	{
		m_start = TClock::now();
		m_duration = 0;
	}

	void reset() noexcept
	{
#if defined min
#	undef min
#endif
		m_start = std::chrono::time_point_cast<Resolution>( TClock::time_point::min() );
	}

	/// \brief	returns the duration between m_start and now in milliseconds
	constexpr size_t getMilliSecondsFromStart() noexcept
	{
		if constexpr( std::is_same_v<Resolution, std::chrono::nanoseconds> )
		{
			m_duration = static_cast<size_t>( std::chrono::duration_cast<Resolution>( TClock::now() - m_start ).count() / 1000000 );
		}
		else if constexpr( std::is_same_v<Resolution, std::chrono::microseconds> )
		{
			m_duration = static_cast<size_t>( std::chrono::duration_cast<Resolution>( TClock::now() - m_start ).count() / 1000 );
		}
		else if constexpr( std::is_same_v<Resolution, std::chrono::milliseconds> )
		{
			m_duration = static_cast<size_t>( std::chrono::duration_cast<Resolution>( TClock::now() - m_start ).count() );
		}
		else if constexpr( std::is_same_v<Resolution, std::chrono::seconds> )
		{
			m_duration = static_cast<size_t>( std::chrono::duration_cast<Resolution>( TClock::now() - m_start ).count() * 1000 );
		}

#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Duration from start (ms): "s + std::to_string( m_duration ) + "\n" );
#endif

		return this->m_duration;
	}

	/// \brief	returns the duration between m_start and now in seconds
	constexpr float getSecondsFromStart() noexcept
	{
		float ret;
		if constexpr( std::is_same_v<Resolution, std::chrono::nanoseconds> )
		{
			ret = static_cast<float>( std::chrono::duration_cast<Resolution>( TClock::now() - m_start ).count() / 1000000000.0f );
		}
		else if constexpr( std::is_same_v<Resolution, std::chrono::microseconds> )
		{
			ret = static_cast<float>( std::chrono::duration_cast<Resolution>( TClock::now() - m_start ).count() / 1000000.0f );
		}
		else if constexpr( std::is_same_v<Resolution, std::chrono::milliseconds> )
		{
			ret = static_cast<float>( std::chrono::duration_cast<Resolution>( TClock::now() - m_start ).count() / 1000.0f );
		}
		else if constexpr( std::is_same_v<Resolution, std::chrono::seconds> )
		{
			ret = static_cast<float>( std::chrono::duration_cast<Resolution>( TClock::now() - m_start ).count() );
		}

#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Duration from start (sec): "s + std::to_string( ret ) + "\n" );
#endif

		return ret;
	}

	/// \brief	marks a lap, which marks the current time
	/// \brief	returns duration in targeted Time Resolution between successive calls of the function and
	float getTimeElapsed() noexcept
	{
		const auto old = m_last;
		m_last = TClock::now();
		return static_cast<float>( std::chrono::duration_cast<Resolution>( m_last - old ).count() );
	}

	/// \brief	marks a lap
	/// \brief	returns duration in seconds
	float getTimeElapsedSecs() noexcept
	{
		const auto old = m_last;
		m_last = TClock::now();
		float ret;
		if constexpr( std::is_same_v<Resolution, std::chrono::nanoseconds> )
		{
			ret = static_cast<float>( std::chrono::duration_cast<Resolution>( m_last - old ).count() / 1000000000.0f );
		}
		else if constexpr( std::is_same_v<Resolution, std::chrono::microseconds> )
		{
			ret = static_cast<float>( std::chrono::duration_cast<Resolution>( m_last - old ).count() / 1000000.0f);
		}
		else if constexpr( std::is_same_v<Resolution, std::chrono::milliseconds> )
		{
			ret = static_cast<float>( std::chrono::duration_cast<Resolution>( m_last - old ).count() / 1000.0f);
		}
		else if constexpr( std::is_same_v<Resolution, std::chrono::seconds> )
		{
			ret = static_cast<float>( std::chrono::duration_cast<Resolution>( m_last - old ).count() );
		}
		return ret;
	}

	/// \brief	returns duration between the last call of getTimeElapsed() and now
	float peekTimeElapsed() const noexcept
	{
		return static_cast<float>( std::chrono::duration_cast<Resolution>( TClock::now() - m_last ).count() );
	}

	/// \brief	works like getTimeElapsed/getTimeElapsedSecs but may be more performant
	float lap() noexcept
	{
		const auto old = m_last;
		m_last = TClock::now();
		const std::chrono::duration<float> dur = m_last - old;
		return dur.count();
	}

#pragma region sleepFor
	/// \brief	delays execution of this_thread for the amount of time specified (in milliseconds, or microseconds)
	static void sleepFor( const float t )
	{
		const auto ms = static_cast<std::uint64_t>( t );
		sleepFor( std::chrono::milliseconds{ms} );
		//const auto us = util::getFractionalPartAsInt( t, 3 );
		//sleepFor( std::chrono::microseconds{us} );
	}
	static void sleepFor( const int t )
	{
		sleepFor( std::chrono::milliseconds{static_cast<uint64_t>( t )} );
	}
	static void sleepFor( const uint64_t t )
	{
		sleepFor( std::chrono::milliseconds{t} );
	}
	static void sleepFor( const std::chrono::milliseconds &t )
	{
		std::this_thread::sleep_for( t );
	}
	static void sleepFor( const std::chrono::microseconds &t )
	{
		std::this_thread::sleep_for( t );
	}
#pragma endregion
};

using SleepTimer = KeyTimer<std::chrono::milliseconds>;