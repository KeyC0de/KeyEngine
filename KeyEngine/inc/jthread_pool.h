#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "key_exception.h"
#include "jthread.h"


//============================================================
//	\class	ThreadPoolEx
//	\author	KeyC0de
//	\date	25/9/2019 3:55
//	\brief	A class which encapsulates a Queue of Tasks & a Pool of threads and dispatches work on demand
//				ie. upon an incoming Task - callable object - a thread is dispatched to execute it
//			Singleton
//			move only
//			Leverages jthread facilities - interruptibility.
//			The functions you enqueue to ThreadPoolJ must have a first argument of `stop_token`
//			WARNING: Remember to call resetInstance before you terminate your program
// example usage:
//		threadPool.enqueue( &doNothing );
//		threadPool.enqueue( &func_async::doNothingForEverUntilStoppedProperly );
//		threadPool.enqueue( &func1 );
//		threadPool.enqueue( &func_async::doPeriodically, &func2, 1000, false );
//=============================================================
class ThreadPoolJ final
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

	static ThreadPoolJ& instance( const std::size_t nThreads, const bool bEnabled );
	static void resetInstance() noexcept;

	//	\function	start	||	\date	25/9/2019 12:20
	//	\brief	calls run
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

//	\function	doPeriodically	||	\date	2021/09/06 1:05
//	\brief	like a timer event --- executes void(*f)() function at periodic (ms) intervals
//	\arg	now : if you want to execute first up now
void doPeriodically( nonstd::stop_token st, const std::function<void(void)> &f, const size_t intervalMs, const bool now );
void doLater( nonstd::stop_token st, const std::function<void(void)> &f, const size_t intervalMs );


}// namespace func_async