#pragma once

#include <vector>
#include <iterator>
#include <type_traits>
#include "assertions_console.h"
#include "algorithms.h"


template<typename T, typename = std::enable_if_t<!std::is_abstract_v<T>>>
class Subscriber;

//=============================================================
//	\class	Subject<T>
//
//	\author	KeyC0de
//	\date	2022/08/27 23:46
//
//	\brief	Subjects of type Subject<T> are interested in being notified of events of type T
//			Subjects are implemented via CRTP
//=============================================================
template<typename T, typename = std::enable_if_t<!std::is_abstract_v<T>>>
class Subject
{
	friend class Subscriber<T>;
	
	std::vector<Subscriber<T>*> m_subscribers;
public:
	Subject() = default;

	virtual ~Subject()
	{
		removeAllSubscribers();
	}

	void removeAllSubscribers()
	{
		auto it = m_subscribers.begin();
		while ( it != m_subscribers.end() )
		{
			Subscriber<T> *pSubscriber = *it;
			util::removeByBackSwap( m_subscribers,
				it );
			// this unsubscribeFromSubject call actually invalidates our iterator
			if ( pSubscriber )
			{
				pSubscriber->unsubscribeFromSubject( this );
			}
			it = m_subscribers.begin();
		}
	}

	void removeSubscriber( const Subscriber<T> *pSubscriber,
		bool bSubscriberRequested = false )
	{
		ASSERT( pSubscriber, "Subscriber was null!" );
		std::vector<Subscriber<T>*>::iterator it = std::find( m_subscribers.begin(),
			m_subscribers.end(),
			pSubscriber );
		if ( it != m_subscribers.end() )
		{
			Subscriber<T> *pTargetSubscriber = *it;
			util::removeByBackSwap( m_subscribers,
				it );
			if ( pTargetSubscriber && !bSubscriberRequested )
			{
				pTargetSubscriber->unsubscribeFromSubject( this );
			}
		}
	}

	//===================================================
	//	\function	notify
	//	\brief  the Subject subclass must call this `notify()` to notify all its subscribers of an event T
	//	\date	2022/08/27 19:33
	void notify( const T& )
	{
		for ( auto it = m_subscribers.begin(); it != m_subscribers.end(); ++it )
		{
			auto* p = *it;
			if ( p )
			{
				p->onNotify( static_cast<T*>( this ) );
			}
		}
	}

protected:
	Subject( const Subject &rhs )
	{
		*this = rhs;
	}

	Subject& operator=( const Subject &rhs )
	{
		for ( auto it = rhs.m_subscribers.begin(); it != rhs.m_subscribers.end(); ++it )
		{
			( *it )->subscribe( this );
		}
		return *this;
	}
private:
	virtual void addSubscriber( const Subscriber<T> *pSubscriber )
	{
		ASSERT( pSubscriber, "Subscriber was null!" );
		// if the subscriber isn't already in our list then add it
		std::vector<Subscriber<T>*>::iterator it = std::find( m_subscribers.begin(),
			m_subscribers.end(),
			pSubscriber );
		if ( it == m_subscribers.end() )
		{
			m_subscribers.emplace_back( pSubscriber );
		}
	}
};

//=============================================================
//	\class	Subscriber<T>
//
//	\author	KeyC0de
//	\date	2022/08/27 19:22
//
//	\brief	inherit from Subscriber using protected inheritance
//			as such the concrete class is implemented in-terms-of-this Subscriber
//			Subscribers cannot unsubscribe by themselves, they have to be X'd by the Subject
//=============================================================
template<typename T, typename = std::enable_if_t<!std::is_abstract_v<T>>>
class Subscriber
{
	std::vector<Subject<T>*> m_subjects;
public:
	Subscriber() = default;

	virtual ~Subscriber() noexcept
	{
		unsubscribeAll();
	}

	void subscribe( Subject<T> *pSubject ) cond_noex
	{
		ASSERT( pSubject, "Subject was null!" );
		// if the subject isn't already in our list then add it
		std::vector<Subject<T>*>::iterator it = std::find( m_subjects.begin(),
			m_subjects.end(),
			pSubject );
		if ( it == m_subjects.end() )
		{
			pSubject->addSubscriber( this );
			m_subjects.emplace_back( pSubject );
		}
	}

	bool isSubscribedTo( const Subject<T> *pSubject ) const noexcept
	{
		ASSERT( pSubject, "Subject was null!" );
		std::vector<Subject<T>*>::iterator it = std::find( m_subjects.begin(),
			m_subjects.end(),
			pSubject );
		if ( it != m_subjects.end() )
		{
			return true;
		}
		return false;
	}

	void unsubscribeFromSubject( const Subject<T> *pSubject )
	{
		ASSERT( pSubject, "Subject was null!" );
		std::vector<Subject<T>*>::iterator it = std::find( m_subjects.begin(),
			m_subjects.end(),
			pSubject );
		if ( it != m_subjects.end() )
		{
			const Subject<T> *pTargetSubject = *it;
			util::removeByBackSwap( m_subjects,
				it );
			if ( pTargetSubject )
			{
				pTargetSubject->removeSubscriber( this,
					true );
			}
		}
	}

	void unsubscribeAll()
	{
		auto it = m_subjects.begin();
		while ( it != m_subjects.end() )
		{
			Subject<T> *pSubject = *it;
			util::removeByBackSwap( m_subjects,
				it );
			if ( pSubject )
			{
				unsubscribeFromSubject( pSubject );
			}
			it = m_subjects.begin();
		}
	}

	//===================================================
	//	\function	onNotify
	//	\brief  The concrete Subscribers must implement the onNotify() method.
	//			onNotify() must be called only by the base Subject
	//	\date	2022/08/27 19:32
	virtual void onNotify( const T &a ) const = 0;
protected:
	Subscriber( const Subscriber &rhs )
	{
		*this = rhs;
	}

	Subscriber& operator=( const Subscriber &rhs )
	{
		for ( auto it = rhs.m_subjects.begin(); it != rhs.m_subjects.end(); ++it )
		{
			subscribe( *it );
		}
		return *this;
	}
};


// A `Subscriber` (subscriber/reporter) may even observe multiple `Subject`s (listeners) of different types:
/*
class EnvironmentWindow
	: public Subscriber<Temperature>,	// Yes Observers work with CRTP
	public Subscriber<Pressure>
{
public:
	EnvironmentWindow()
	{
		
	}

	~EnvironmentWindow()
	{
		
	}

	// must implement both onNotify base class abstract functions
	void onNotify( Temperature *subject )
	{
		std::cout << "Temperature was changed" << std::endl;
		subject->getTemperature();
	}

	void onNotify( Pressure *subject )
	{
		std::cout << "Pressure was changed" << std::endl;
		subject->getPressure();
	}
};
*/