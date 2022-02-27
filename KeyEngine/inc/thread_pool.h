#pragma once

#include <future>
#include <queue>
#include <memory>

#define M_ENABLED m_bEnabled.load( std::memory_order_relaxed )


//============================================================
//	\class	ThreadPool
//
//	\author	KeyC0de
//	\date	25/9/2019 3:55
//
//	\brief	A class which encapsulates a Queue of Tasks & a Pool of threads
//				and dispatches work on demand - ie. upon an incoming Task - callable object -
//				a thread is dispatched to execute it
//			Singleton, move only class
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
	explicit ThreadPool( std::size_t nthreads, bool bStart = true );
public:	
	~ThreadPool() noexcept;
	ThreadPool( ThreadPool const& ) = delete;
	ThreadPool& operator=( const ThreadPool& rhs ) = delete;
	ThreadPool( ThreadPool&& rhs ) noexcept;
	ThreadPool& operator=( ThreadPool&& rhs ) noexcept;

	static ThreadPool& getInstance( std::size_t nThreads
		= std::thread::hardware_concurrency(), bool bEnabled = true );
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
	decltype( auto ) enqueue( Callback&& f,
		TArgs&&... args )
	{
		using ReturnType = std::invoke_result_t<Callback, TArgs...>;
		using FuncType = ReturnType(TArgs...);
		using Wrapped = std::packaged_task<FuncType>;

		if ( M_ENABLED )
		{
			std::shared_ptr<Wrapped> smartFunctionPointer =
				std::make_shared<Wrapped>( std::move( f ) );
			std::future<ReturnType> fu = smartFunctionPointer->get_future();

			auto task = [smartFunctionPointer = std::move( smartFunctionPointer ),
					args = std::make_tuple( std::forward<TArgs>( args )... )] () -> void
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
			// TODO: rework exception handling
			throw std::exception{"Cannot enqueue tasks in an inactive Thread Pool!"};
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