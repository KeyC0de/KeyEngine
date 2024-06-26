#pragma once

#include <vector>
#include "non_copyable.h"
#include "assertions_console.h"


class ReporterAccess;

template<typename T> 
class IReporter;

//=============================================================
//	\class	IListener<T>
//	\author	KeyC0de
//	\date	2024/05/07 22:04
//	\brief	Templated Observer design pattern (Class Template 1 of 2)
// 
//		To become a listener of 1 or more events T, U, V etc. a class ("MyClass") must do the following:
//		1. Inherit from IListener:
//			class MyClass
//			: public MyOtherClass,
//			public IListener<T>,	// eg. T = StartedTemperatureMeasurements
//			public IListener<U>,	// eg. U = ObjectDestroyed<Instrumentation>
//			...
//		2. add a `void notify( const T &event ) override` method for every single event listener IListener<T>, IListener<U>, ... you inherit from
//		The purpose of the nofify method is for MyClass to respond to the event appropriately.
//		eg. notify( const StartedTemperatureMeasurements &event )
//		3. the event types T, U, V etc. themselves can be created as simple structs.
//		eg.
//			struct StartedTemperatureMeasurements
//			{
//				StartedTemperatureMeasurements( const float time, const bool hasStarted, const std::string &temperatureType = "Celsius")
//					:
//					m_time{time},
//					m_temperatureType{temperatureType},
//					m_hasStarted{hasStarted}
//				{}
// 
//				float m_time;
//				std::string m_temperatureType;
//				bool m_hasStarted;
//			};
//		or
//			template<typename T>
//			struct ObjectDestroyed
//			{
//				ObjectDestroyed( const T& object )
//					: m_object{object}
//				{}
// 
//				const T& m_object;
//			};
//		4. to attach the listener you must get a hold of a ReportingNexus, which is a class that inherits from all IReporters.
//		You can even group IReporters into different ReportingNexus classes.
//		Once you have a ReportingNexus object cast it to the proper IReporter sub-class and .addListener for every IListener your class is listening to:
//		static_cast<const IReporter<T>&>( reportingNexus ).addListener( this );
//		eg.
//			MyClass::MyClass( ... )
//				: ...
//			{
//				...
//				static_cast<const IReporter<StartedTemperatureMeasurements>&>( reportingNexus ).addListener( this );
//				static_cast<const IReporter<ObjectDestroyed<Instrumentation>>&>( reportingNexus ).addListener( this );
//				...
//			}
//=============================================================
template<typename T> 
class IListener
	: public NonCopyableAndNonMovable
{
	friend class IReporter<T>;

	mutable std::vector<const void*> m_reporters;
public:
	virtual ~IListener()
	{
		auto it = m_reporters.begin();
		while ( it != m_reporters.end() )
		{
			const IReporter<T> *reporter = reinterpret_cast<const IReporter<T>*>( *it );
			it = m_reporters.erase( it );
			reporter->removeListener( this );	// this removeListener call actually invalidates our iterator
		}
	}

	virtual void notify( const T& event ) = 0;
private:
	void addReporter( const IReporter<T> *reporter )
	{
		// check for repeated entries in IReporter::addListener instead of here
		m_reporters.push_back( reinterpret_cast<const void*>( reporter ) );
	}
};


//=============================================================
//	\class	IReporter<T>
//	\author	KeyC0de
//	\date	2024/05/07 22:49
//	\brief	Templated Observer design pattern (Class Template 2 of 2)
//
//		To become an IReporter of an event T, U, V a class ("MyClass") must do the following:
//		1. Inherit from IReporter:
//			class MyClass
//				: ...
//				public IReporter<T>	// eg. T = StartedTemperatureMeasurements
//		2. make sure all inheriting concrete classes implement a destructor which calls `removeThisFromListenersList()` as the last instruction, for all the IReporter's it inherited from.
//		`removeThisFromListenersList()`
//		`IReporter<T>::removeThisFromListenersList()`
//		If a derived class doesn't implement a destructor it will be an abstract class.
//		3. at some point MyClass will want to report on the event T, which will trigger the `notify( const T& )` method of all of its IListeners<T>.
//		To do that you must get a hold of a ReportingNexus object, cast it to the proper IReporter sub-class and finally:
//		static_cast<IReporter<T>&>( reportingNexus ).notifyListeners( T( /*ctor arguments for event T*/ ) );	// the cast is necessary only if MyClass derives from multiple IReporter<>s.
//		eg.
//			static_cast<IReporter<StartedTemperatureMeasurements>&>( *this ).notifyListeners( StartedTemperatureMeasurements( m_measurements ) );
//			IReporter<ObjectDestroyed<MyClass>>::notifyListeners( ObjectDestroyed<MyClass>( *this ) );
//			IReporter<ObjectDestroyed<Instrumentation>>::notifyListeners( ObjectDestroyed<Instrumentation>( *m_thermometer ) );
//=============================================================
template<typename T> 
class IReporter
	: public NonCopyableAndNonMovable
{
	mutable unsigned short m_notifyCounter;
	mutable std::vector<void*> m_listeners;	// pointers to objects of type Listener<T> interested in being notified in events of type T
	mutable std::vector<void*> m_listenersPendingAdd;
public:
	// force construction of reporters with an ReporterAccess to prevent objects without a permissions from using this system
	explicit IReporter( ReporterAccess & )
		:
		m_notifyCounter(0),
		m_listeners(),
		m_listenersPendingAdd()
	{

	}

	virtual ~IReporter() noexcept = 0;

	bool hasListener( IListener<T> *listener ) const
	{
		return std::find( m_listeners.begin(), m_listeners.end(), reinterpret_cast<void*>( listener ) ) != m_listeners.end()
			|| std::find( m_listenersPendingAdd.begin(), m_listenersPendingAdd.end(), reinterpret_cast<void*>( listener ) ) != m_listenersPendingAdd.end();
	}

	void addListener( IListener<T> *listener ) const
	{
		const bool result = tryAddListener( listener );
		ASSERT( result, "Attempting to add an object as a listener that is already in the listeners list!" );
	}

	//	\function	removeListener	||	\date	2024/05/08 0:23
	//	\brief	called from ~IListener
	void removeListener( const IListener<T> *listener ) const
	{
		// mark for removal at next `notifyListeners` call
		auto it = std::find( m_listeners.begin(), m_listeners.end(), reinterpret_cast<const void*>( listener ) );
		if ( it != m_listeners.end() )
		{
			*it = nullptr;
		}

		// remove from pending list
		m_listenersPendingAdd.erase( std::remove( m_listenersPendingAdd.begin(), m_listenersPendingAdd.end(), reinterpret_cast<const void*>( listener ) ), m_listenersPendingAdd.end() );

		// remove ourselves from the listener's reporter list
		listener->m_reporters.erase( std::remove( listener->m_reporters.begin(), listener->m_reporters.end(), reinterpret_cast<const void*>( this ) ), listener->m_reporters.end() );
	}

	void notifyListeners( const T &obj )
	{
#if !defined( FINAL_RELEASE )
		static bool bReentrancyCheck = false;
		ASSERT( bReentrancyCheck == false, "Detected possible reentrancy in IReporter::notifyListeners - this may lead to a crash." );
		bReentrancyCheck = true;
#endif//!FINAL_RELEASE

		if ( m_notifyCounter == 0 )
		{
			// it's safe to process pending adds at this point
			m_listeners.insert( m_listeners.end(), m_listenersPendingAdd.begin(), m_listenersPendingAdd.end() );
			m_listenersPendingAdd.clear();
		}

		++m_notifyCounter;

		for ( std::vector<void*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it )
		{
			IListener<T> *listener = reinterpret_cast<IListener<T>*>( *it );
			if ( listener == nullptr )
			{
				continue;
			}

			listener->notify( obj );
		}

		--m_notifyCounter;
		if ( m_notifyCounter == 0 )
		{
			// remove invalids
			m_listeners.erase( std::remove( m_listeners.begin(), m_listeners.end(), nullptr ), m_listeners.end() );
		}

#if !defined( FINAL_RELEASE )
		bReentrancyCheck = false;
#endif//!FINAL_RELEASE
	}

	void notifyListeners( T &&reporter )
	{
		notifyListeners( static_cast<const T&>( reporter ) );
	}

	template<typename... TArgs>
	void notifyListeners( TArgs &&...args )
	{
		notifyListeners( T( std::forward<TArgs>( args )... ) );
	}

	bool anyListenersPresent() const
	{
		return !m_listeners.empty() || !m_listenersPendingAdd.empty();
	}

	int numListeners() const
	{
		return m_listeners.size() + m_listenersPendingAdd.size();
	}
protected:
	void removeThisFromListenersList()
	{
		for ( std::vector<void*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it )
		{
			IListener<T> *listener = reinterpret_cast<IListener<T>*>( *it );
			if ( listener != nullptr )
			{
				listener->m_reporters.erase( std::remove( listener->m_reporters.begin(), listener->m_reporters.end(), reinterpret_cast<const void*>( this ) ), listener->m_reporters.end() );
			}
		}

		for ( std::vector<void*>::iterator it = m_listenersPendingAdd.begin(); it != m_listenersPendingAdd.end(); ++it )
		{
			IListener<T> &listener = *reinterpret_cast<IListener<T>*>( *it );
			listener.m_reporters.erase( std::remove( listener.m_reporters.begin(), listener.m_reporters.end(), reinterpret_cast<const void*>( this ) ), listener.m_reporters.end() );
		}
	}
private:
	bool tryAddListener( IListener<T> *listener ) const
	{
		// check for repeated entries here (and not in IReporter::addReporter)
		if ( !hasListener( listener ) )
		{
			m_listenersPendingAdd.push_back( reinterpret_cast<void*>( listener ) );
			listener->addReporter( this );
			return true;
		}
		return false;
	}
};


template<typename T>
IReporter<T>::~IReporter() noexcept
{

}
