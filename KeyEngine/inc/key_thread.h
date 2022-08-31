#include <thread>
#include <future>
#include "non_copyable.h"


class Flag final
{
	bool m_flag = false;
public:
	void set();
	void reset();
	bool isSet() const noexcept;
};

//=============================================================
//	\class	KeyThread
//
//	\author	KeyC0de
//	\date	2021/07/07 19:13
//
//	\brief	jthread wannabe, 1. interruptibility & 2. automatic cleanup
//			KeyThread stores a pointer to a thread_local flag that we can interrupt at will
//			we need to set a thread specific interrupt flag
//			we can't have thread local members but we can have static thread local members
//				and we connect object with thread via the static thread local flag
//				and to do this we use future-promise in the ctor
//=============================================================
class KeyThread
	: public NonCopyable
{
private:
	std::thread m_thread;
	Flag *m_pInterruptFlag;
public:
	KeyThread() = delete;

	template<typename F>
	KeyThread( F f )
	{
		std::promise<Flag*> prom;
		m_thread = std::thread{
			[f, &prom] ()
			{
				prom.set_value( &threadFlag );
				f();
			}
		};
		m_pInterruptFlag = prom.get_future().get();
	}

	~KeyThread() noexcept;

	//===================================================
	//	\function	interrupt
	//	\brief  will be called by the main or another thread
	//	\date	2021/07/07 20:51
	void interrupt();
	bool isInterrupted() const noexcept;
public:
	// creates a different flag for each thread
	inline static thread_local Flag threadFlag;

	static bool isThreadInterrupted() noexcept
	{
		if ( threadFlag.isSet() )
		{
			return true;
		}
		return false;
	}
};