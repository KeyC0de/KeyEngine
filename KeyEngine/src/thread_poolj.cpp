#include "thread_poolj.h"
#include "assertions_console.h"


ThreadPoolJ::ThreadPoolJ( const std::size_t nthreads,
	const bool bStart )
{
	m_pool.reserve( nthreads );
	if ( bStart )
	{
		start();
	}
}

ThreadPoolJ& ThreadPoolJ::getInstance( const std::size_t nThreads /*= 4u*/,
	const bool bEnabled /*= true*/ )
{
	std::unique_lock<std::recursive_mutex> lg{s_mu};
	if ( s_pInstance == nullptr )
	{
		s_pInstance = new ThreadPoolJ{nThreads, bEnabled};
	}
	return *s_pInstance;
}

void ThreadPoolJ::resetInstance() noexcept
{
	std::unique_lock<std::recursive_mutex> lg{s_mu};
	if ( s_pInstance )
	{
		delete s_pInstance;
		s_pInstance = nullptr;
	}
}

ThreadPoolJ::~ThreadPoolJ() noexcept
{
	stop();
	m_pool.clear();	// needed for "extraordinary circumstances"
}

ThreadPoolJ::ThreadPoolJ( ThreadPoolJ &&rhs ) noexcept
	:
	m_pool{std::move( rhs.m_pool )},
	m_tasks{std::move( rhs.m_tasks )}
{
	m_bEnabled = rhs.m_bEnabled;
	rhs.m_bEnabled = false;
}

ThreadPoolJ& ThreadPoolJ::operator=( ThreadPoolJ &&rhs ) noexcept
{
	ThreadPoolJ tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

void ThreadPoolJ::start()
{
	if ( !m_bEnabled )
	{
		enable();

		run();
	}
}

void ThreadPoolJ::stop() noexcept
{
	if ( m_bEnabled )
	{
		disable();

		m_cond.notify_all();
		for ( auto &t : m_pool )
		{
			ASSERT( t.get_stop_token().stop_possible(), "Stop is not possible!" )
			t.request_stop();
			if ( t.joinable() )
			{
				t.join();
			}
		}
	}
}

void ThreadPoolJ::enable() noexcept
{
	m_bEnabled = true;
}

void ThreadPoolJ::disable() noexcept
{
	m_bEnabled = false;
}

bool ThreadPoolJ::isEnabled() const noexcept
{
	return m_bEnabled;
}

void ThreadPoolJ::run()
{
	std::size_t nthreads = m_pool.capacity();

	// this `stop_token st` is jthread's own stop_token (not some random argument being passed to the thread's function)
	auto threadMain = [this] ( nonstd::stop_token st ) -> void
	{
		// thread sleeps forever until there's a task available
		while ( true )
		{
			std::unique_lock<std::mutex> ul{m_mu};
			while ( m_tasks.empty() && m_bEnabled )
			{
				m_cond.wait( ul );
			}

			if ( !m_bEnabled )
			{
				break;
			}

			if ( !m_tasks.empty() )
			{
				Task task = std::move( m_tasks.front() );
				m_tasks.pop();
				ul.unlock();
				task( st );
			}
		}
	};

	for ( std::size_t ti = 0; ti < nthreads; ++ti )
	{
		m_pool.emplace_back( nonstd::jthread{std::move( threadMain )});
	}
}


namespace func_async
{

void doPeriodically( nonstd::stop_token st,
	const std::function<void(void)> &f,
	const size_t intervalMs,
	const bool now )
{
	if ( now )
	{
		while ( !st.stop_requested()  )
		{
			f();
			auto chronoInterval = std::chrono::milliseconds( intervalMs );
		}
	}
	else
	{
		while ( !st.stop_requested() )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( intervalMs ) );
			f();
		}
	}
}

void doLater( nonstd::stop_token st,
	const std::function<void(void)> &f,
	const size_t intervalMs )
{
	std::this_thread::sleep_for( std::chrono::milliseconds( intervalMs ) );
	f();
}

}// namespace func_async