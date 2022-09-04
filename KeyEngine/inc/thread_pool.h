#pragma once

#include <future>
#include <queue>
#include <memory>
#include "key_exception.h"

#define M_ENABLED m_bEnabled.load( std::memory_order_relaxed )


//============================================================
//	\class	ThreadPool
//
//	\author	KeyC0de
//	\date	25/9/2019 3:55
//
//	\brief	A class which encapsulates a Queue of Tasks & a Pool of threads and dispatches work on demand
//				ie. upon an incoming Task - callable object - a thread is dispatched to execute it
//			Singleton, move only class
//			WARNING: Remember to call resetInstance before you terminate your program
//=============================================================
class ThreadPool final
{
	using Task = std::function<void()>;

	static inline ThreadPool *m_pInstance;
	static inline std::mutex ms_mu;

	std::atomic<bool> m_bEnabled;
	std::vector<std::thread> m_pool;
	std::queue<Task> m_tasks;
	std::condition_variable m_cond;
	std::mutex m_mu;
private:
	explicit ThreadPool( const std::size_t nthreads, const bool bStart = true );
public:
	~ThreadPool() noexcept;
	ThreadPool( ThreadPool &&rhs ) noexcept;
	ThreadPool& operator=( ThreadPool &&rhs ) noexcept;

	static ThreadPool& instance( const std::size_t nThreads = std::thread::hardware_concurrency(), const bool bEnabled = true );
	static void resetInstance();

	//===================================================
	//	\function	start
	//	\brief  calls run
	//	\date	25/9/2019 12:20
	void start();
	void stop() noexcept;
	void enable() noexcept;
	void disable() noexcept;
	bool isEnabled() const noexcept;

	template<typename Callback, typename... TArgs>
	decltype( auto ) enqueue( Callback &&f,
		TArgs &&...args )
	{
		using ReturnType = std::invoke_result_t<Callback, TArgs...>;
		using TFunc = ReturnType(TArgs...);
		using TFWrapped = std::packaged_task<TFunc>;

		if ( M_ENABLED )
		{
			std::shared_ptr<TFWrapped> smartFunctionPointer = std::make_shared<TFWrapped>( std::move( f ) );
			std::future<ReturnType> fu = smartFunctionPointer->get_future();

			auto task = [smartFunctionPointer = std::move( smartFunctionPointer ),
					args = std::make_tuple( std::forward<TArgs>( args )... )]
				() -> void
				{
					std::apply( *smartFunctionPointer,
						std::move( args ) );
					return;
				};

			{
				std::lock_guard<std::mutex> lg{m_mu};
				m_tasks.emplace( std::move( task ) );
			}
			m_cond.notify_one();
			return fu;
		}
		else
		{
			THROW_KEY_EXCEPTION( "Cannot enqueue tasks in an inactive Thread Pool!" );
		}
	}
	//===================================================
	//	\function	resize
	//	\brief  adds # or subtracts -# threads to the ThreadPool
	//	\date	25/9/2019 4:00
	bool resize( int n );
private:
	void run();
};