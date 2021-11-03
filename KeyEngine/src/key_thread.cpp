#include "key_thread.h"
#if defined _DEBUG && !defined NDEBUG
#	include <iostream>
#endif


void Flag::set()
{
	m_flag = true;
}

void Flag::reset()
{
	m_flag = false;
}

bool Flag::isSet() const noexcept
{
	return m_flag;
}


KeyThread::~KeyThread() noexcept
{
	if ( m_thread.joinable() )
	{
		m_thread.join();
	}
}

void KeyThread::interrupt()
{
#if defined _DEBUG && !defined NDEBUG
	std::cout << "thread "
		<< std::this_thread::get_id()
		<< " interrupted!\n";
#endif
	m_pInterruptFlag->set();
}

bool KeyThread::isInterrupted() const noexcept
{
	return m_pInterruptFlag->isSet();
}