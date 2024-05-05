#pragma once

#include <future>
#include <queue>
#include <memory>
#include "key_exception.h"

#define M_ENABLED m_bEnabled.load( std::memory_order_relaxed )


//============================================================
//	\class	ThreadPool
//	\author	KeyC0de
//	\date	25/9/2019 3:55
//	\brief	A class which encapsulates a Queue of Tasks & a Pool of threads and dispatches work on demand
//				ie. upon an incoming Task - callable object - a thread is dispatched to execute it
//			Singleton class
//			move only
//=============================================================
class ThreadPool final
{
	using Task = std::function<void()>;

	std::atomic<bool> m_bEnabled;
	std::vector<std::thread> m_pool;
	std::queue<Task> m_tasks;
	std::condition_variable m_cond;
	std::mutex m_mu;
private:
	explicit ThreadPool( const std::size_t nthreads, const bool bStart = true );
public:
	~ThreadPool() noexcept;
	ThreadPool( ThreadPool const &rhs ) = delete;
	ThreadPool& operator=( const ThreadPool &rhs ) = delete;
	ThreadPool( ThreadPool &&rhs ) noexcept;
	ThreadPool& operator=( ThreadPool &&rhs ) noexcept;

	static ThreadPool& getInstance( const std::size_t nThreads = std::thread::hardware_concurrency(), const bool bEnabled = true );
	//	\function	start	||	\date	25/9/2019 12:20
	//	\brief	calls run
	void start();
	void stop() noexcept;
	void enable() noexcept;
	void disable() noexcept;
	bool isEnabled() const noexcept;

	template<typename Callback, typename... TArgs>
	decltype( auto ) enqueue( Callback &&f,
		TArgs &&... args )
	{
		using ReturnType = std::invoke_result_t<Callback, TArgs...>;
		using FunctionType = ReturnType(TArgs...);

		if ( M_ENABLED )
		{
			std::shared_ptr<std::packaged_task<FunctionType>> smartFunctionPointer = std::make_shared<std::packaged_task<FunctionType>>( std::move( f ) );
			std::future<ReturnType> fu = smartFunctionPointer->get_future();

			// *trick* for pre-C++20 to call a variadic function with vararg params, bind the params in a tuple and call it by std::apply
			// packaged_task converts into std::function automatically
			// and we need shared_ptr to pass it properly to the lambda capture for it to work
			auto task = [smartFunctionPointer = std::move( smartFunctionPointer ),
				args = std::make_tuple( std::forward<TArgs>( args )... )]
				() -> void
			{
				std::apply( *smartFunctionPointer, std::move( args ) );
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
	//	\function	resize	||	\date	25/9/2019 4:00
	//	\brief	adds # or subtracts -# threads to the ThreadPool
	bool resize( int n );
private:
	void run();
};