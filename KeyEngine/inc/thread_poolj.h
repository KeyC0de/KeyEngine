#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "jthread/jthread.h"
#include "jthread/stop_token.h"
#include "non_copyable.h"
#include "key_exception.h"


///============================================================
/// \class	ThreadPoolJ
/// \author	KeyC0de
/// \date	25/9/2019 3:55
/// \brief	A class which encapsulates a Queue of Tasks & a Pool of threads and dispatches work on demand
/// \brief		ie. upon an incoming Task - callable object - a thread is dispatched to execute it
/// \brief		singleton class
/// \brief	move only
/// \brief	Leverages jthread facilities - interruptibility.
/// \brief	The functions you enqueue to ThreadPoolJ must have a first argument of `stop_token`
/// \brief
/// \brief	WARNING: Remember to call resetInstance before you terminate your program
/// \brief	example usage:
/// \brief		threadPool.enqueue( &doNothing );
/// \brief		threadPool.enqueue( &func_async::doNothingForEverUntilStoppedProperly );
/// \brief		threadPool.enqueue( &func1 );
/// \brief		threadPool.enqueue( &func_async::doPeriodically, &func2, 1000, false );
///=============================================================
class ThreadPoolJ final
	: public NonCopyable
{
	using Task = std::function<void(nonstd::stop_token)>;

	static inline ThreadPoolJ *s_pInstance;
	static inline std::recursive_mutex s_mu;

	bool m_bEnabled = false;
	std::vector<nonstd::jthread> m_pool;
	std::queue<Task> m_tasks;
	std::condition_variable m_cond;
	std::mutex m_mu;
private:
	explicit ThreadPoolJ( const std::size_t nthreads, const bool bStart );
public:
	~ThreadPoolJ() noexcept;
	ThreadPoolJ( const ThreadPoolJ &rhs ) = delete;
	ThreadPoolJ& operator=( const ThreadPoolJ &rhs ) = delete;
	ThreadPoolJ( ThreadPoolJ &&rhs ) noexcept;
	ThreadPoolJ& operator=( ThreadPoolJ &&rhs ) noexcept;

	static ThreadPoolJ& getInstance( const std::size_t nThreads = 4u, const bool bEnabled = true );
	static void resetInstance() noexcept;

	/// \brief	calls run
	void start();
	void stop() noexcept;
	bool isEnabled() const noexcept;

	template<typename TCallable, typename... TArgs>
	void enqueue( TCallable &&f,
		TArgs &&...args )
	{
		if ( m_bEnabled )
		{
			auto task = std::bind( std::move( f ), std::placeholders::_1, std::forward<TArgs>( args )... );
			{
				std::unique_lock<std::mutex> ul{m_mu};
				m_tasks.emplace( std::move( task ) );
			}

			m_cond.notify_one();
		}
		else
		{
			THROW_KEY_EXCEPTION( "Cannot enqueue tasks in an inactive Thread Pool!" );
		}
	}
private:
	void enable() noexcept;
	void disable() noexcept;
	void run();
};


namespace func_async
{

/// \brief	like a timer event --- executes void(*f)() function at periodic (ms) intervals
/// \arg	now : if you want to execute first up now
void doPeriodically( nonstd::stop_token st, const std::function<void(void)> &f, const size_t intervalMs, const bool now );
void doLater( nonstd::stop_token st, const std::function<void(void)> &f, const size_t intervalMs );


}// namespace func_async