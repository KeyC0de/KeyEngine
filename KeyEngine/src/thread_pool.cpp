#include <cstddef>
#include "thread_pool.h"


ThreadPool::ThreadPool( std::size_t nthreads,
	bool bStart )
	:
	m_bEnabled{bStart}
{
	m_pool.reserve( nthreads );
	if ( bStart )
	{
		run();
	}
}

ThreadPool& ThreadPool::getInstance( std::size_t nThreads,
	bool bEnabled )
{
	static ThreadPool instance{nThreads, bEnabled};
	return instance;
}

ThreadPool::~ThreadPool() noexcept
{
	stop();
}

ThreadPool::ThreadPool( ThreadPool&& rhs ) noexcept
	:
	m_bEnabled{std::move( rhs.m_bEnabled.load( std::memory_order_relaxed ) )},
	m_pool{std::move( rhs.m_pool )},
	m_tasks{std::move( rhs.m_tasks )}
{

}

ThreadPool& ThreadPool::operator=( ThreadPool&& rhs ) noexcept
{
	m_bEnabled.store( rhs.m_bEnabled.load( std::memory_order_relaxed ) );
	std::swap( m_pool, rhs.m_pool );
	std::swap( m_tasks, rhs.m_tasks );
	return *this;
}

void ThreadPool::start()
{
	if ( !M_ENABLED )
	{
		m_bEnabled.store( true,
			std::memory_order_relaxed );
		run();
	}
}

void ThreadPool::stop() noexcept
{
	if ( M_ENABLED )
	{
		m_bEnabled.store( false,
			std::memory_order_relaxed );
		m_cond.notify_all();
		for ( auto& t : m_pool )
		{
			if ( t.joinable() )
			{
				t.join();
			}
		}
	}
}

void ThreadPool::enable() noexcept
{
	m_bEnabled.store( true,
		std::memory_order_relaxed );
}

void ThreadPool::disable() noexcept
{
	m_bEnabled.store( false,
		std::memory_order_relaxed );
}

bool ThreadPool::isEnabled() const noexcept
{
	return M_ENABLED;
}

bool ThreadPool::resize( int n )
{
	if ( !isEnabled() )
	{
		return false;
	}
	if ( n > 0 )
	{
		// TODO: add threads
		return true;
	}
	else
	{
		// remove threads
		n = -n;
		{
			std::lock_guard<std::mutex> lg{m_mu};
			if ( m_pool.size() - n < 0 )
			{
				stop();
			}
			else
			{
				for ( size_t i = m_pool.size() - 1; i >= m_pool.size() - n; --i )
				{
					if ( m_pool[i].joinable() )
					{
						m_pool[i].join();
					}
				}
			}
		}
		return true;
	}
}

void ThreadPool::run()
{
	std::size_t nthreads = m_pool.capacity();

	auto threadMain = [this] ()
	{
		// thread sleeps forever until there's a task available
		while( true )
		{
			std::unique_lock<std::mutex> ul{m_mu};
			while( m_tasks.empty() && M_ENABLED )
			{
				m_cond.wait( ul );
			}

			if ( !M_ENABLED )
			{
				break;
			}

			if ( !m_tasks.empty() )
			{
				Task task = std::move( m_tasks.front() );
				m_tasks.pop();
				ul.unlock();
				task();
			}
		}
	};

	for( std::size_t ti = 0; ti < nthreads; ++ti )
	{
		m_pool.emplace_back( std::thread{std::move( threadMain )} );
	}
}