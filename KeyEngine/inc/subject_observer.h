#pragma once

#include <vector>
#include <iterator>
#include <type_traits>
#include "assertions_console.h"
#include "utils.h"


template<typename T, typename = std::enable_if_t<!std::is_abstract_v<T>>>
class ISubscriber;

//=============================================================
//	\class	Subject<T>
//	\author	KeyC0de
//	\date	2022/08/27 23:46
//	\brief	Subject<T>s are interested in being notified of events of type T
//			Subjects are implemented via CRTP
//=============================================================
template<typename T, typename = std::enable_if_t<!std::is_abstract_v<T>>>
class Subject
{
	friend class ISubscriber<T>;

	std::vector<ISubscriber<T>*> m_subscribers;
public:
	Subject() = default;

	virtual ~Subject()
	{
		// for all the subscribers we have remove ourself from their list of subjects
		auto it = m_subscribers.begin();
		while ( it != m_subscribers.end() )
		{
			ISubscriber<T> *pSubscriber = *it;
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

	void removeSubscriber( const ISubscriber<T> *pSubscriber,
		bool bSubscriberRequested = false )
	{
		ASSERT( pSubscriber, "ISubscriber was null!" );
		std::vector<ISubscriber<T>*>::iterator it = std::find( m_subscribers.begin(),
			m_subscribers.end(),
			pSubscriber );
		if ( it != m_subscribers.end() )
		{
			ISubscriber<T> *pTargetSubscriber = *it;
			util::removeByBackSwap( m_subscribers,
				it );
			if ( pTargetSubscriber && !bSubscriberRequested )
			{
				pTargetSubscriber->unsubscribeFromSubject( this );
			}
		}
	}

	//	\function	notify	||	\date	2022/08/27 19:33
	//	\brief	the Subject subclass must call this `notify()` to notify all its subscribers of an event T
	void notify( const T& )
	{
		// take a copy of the list as during notify() some listeners might remove themselves thus invalidating the iterator
		auto subscribers = m_subscribers;
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
	virtual void addSubscriber( const ISubscriber<T> *pSubscriber )
	{
		ASSERT( pSubscriber, "ISubscriber was null!" );
		// if the subscriber isn't already in our list then add it
		std::vector<ISubscriber<T>*>::iterator it = std::find( m_subscribers.begin(),
			m_subscribers.end(),
			pSubscriber );
		if ( it == m_subscribers.end() )
		{
			m_subscribers.emplace_back( pSubscriber );
		}
	}
};

//=============================================================
//	\class	ISubscriber<T>
//	\author	KeyC0de
//	\date	2022/08/27 19:22
//	\brief	inherit from ISubscriber using protected inheritance
//			as such the concrete class is implemented in-terms-of-this ISubscriber
//			Subscribers cannot unsubscribe by themselves, they have to be X'd by the Subject
//=============================================================
template<typename T, typename = std::enable_if_t<!std::is_abstract_v<T>>>
class ISubscriber
{
	std::vector<Subject<T>*> m_subjects;
public:
	ISubscriber() = default;

	virtual ~ISubscriber() noexcept
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

	//	\function	onNotify	||	\date	2022/08/27 19:32
	//	\brief	The concrete Subscribers must implement the onNotify() method.
	//			onNotify() must be called only by the base Subject
	virtual void onNotify( const T &a ) const = 0;
protected:
	ISubscriber( const ISubscriber &rhs )
	{
		*this = rhs;
	}

	ISubscriber& operator=( const ISubscriber &rhs )
	{
		for ( auto it = rhs.m_subjects.begin(); it != rhs.m_subjects.end(); ++it )
		{
			subscribe( *it );
		}
		return *this;
	}
};


/*
Reporter - Listener Pattern
===========================
1. reporter - listener source file
2. class that wants to listen for an event:
				class TurnWidgetSeasons :
								public TWUI::ComponentCallbacks,
								public EMPIREUTILITY::SAFER_LISTENER<EMPIRECAMPAIGN::REPORT_START_OF_TURN_END>,
								public EMPIREUTILITY::LISTENER<sth_else>,
								...
3. add a notify() function for every single listener you inherit from
								void notify(const EMPIRECAMPAIGN::REPORT_START_OF_TURN_END& msg) override;
								void notify(const sth_else& msg) override;
4. The notification/report itself are created as simple structs.
				struct REPORT_START_OF_TURN_END
				{
								REPORT_START_OF_TURN_END(const FACTION&, const String& faction_record, const bool has_pending_battle, bool suppress_shroud_update);
								const FACTION&              m_faction;
								const String& m_faction_record;
								const bool m_has_pending_battle;
								bool m_suppress_shroud_update;
				};
5. add a listener to the Subject/Reporter, in the constructor of the class that's listening:
								static_cast< const SAFER_REPORTER<TAX_LEVELS_SET>&>(local_faction()).add_listener(this);

								void add_listener(SAFER_LISTENER<T>* o) const
								{
												//Check for repeat entries here instead on SAFER_REPORTER::add_reporter
												if(std::find(m_listeners.begin(), m_listeners.end(), reinterpret_cast<void *>(o)) == m_listeners.end())
												{
																m_listeners.push_back(reinterpret_cast<void *>(o));
																o->add_reporter(this);
												}
												else
												{
																CA_ERROR("Attempting to add an object as a listener that is already in the listeners list!");
												}
								}

6. Now for a class method to report on an event:
				struct TAX_LEVELS_SET
				{
								const FACTION &                                              faction;

								TAX_LEVELS_SET(const FACTION& f) : faction(f) {}
				};

				class FACTION : public CAMPAIGN_ENV_MODEL_ACCESS,
																				public COMMAND_QUEUE_INDEXED_OBJECT<FACTION>,
																				public SAFER_REPORTER<TAX_LEVELS_SET>,
																				...
and at some point some method will notify listeners on the event:
								static_cast<SAFER_REPORTER<TAX_LEVELS_SET>&>(*this).notify_listeners(TAX_LEVELS_SET(*this));
AND AFTERWARDS will actually trigger the event:
								EVENT::trigger_event(GOVERNORSHIP_TAX_RATE_CHANGE(*this));

7. Event class:
class EVENT
				{
								friend EMPIREUTILITY_API class EMPIRE_LUA_ENV;
				public:

								enum PARAMETER
								{
												P_NONE,
												P_BATTLE_ALLIANCE,
												P_BATTLE_SHIP_DEPRECATED,
												P_BATTLE_UNIT,
												P_BUILDING_LEVEL,
												P_CHARACTER,
												P_FACTION,
												P_FORT,
												P_GARRISON_RESIDENCE,
												P_HISTORICAL_CHARACTER,
												P_HISTORICAL_EVENT,
												P_MILITARY_FORCE,
												P_MISSION,
												P_MISSION_RESULT,
												P_PORT,
												P_REGION,
												P_REGION_SLOT,
												P_SETTLEMENT,
												P_TARGET_CHARACTER,
												P_TARGET_FACTION,
												P_TECHNOLOGY,
												P_UNIT,
												P_MAP_POSITION,
												P_FACTION_SEA_TRADE_RAIDED,
												P_FACTION_LAND_TRADE_RAIDED,
												P_PROVINCE,

												P_NUM_PARAMETERS
								};

				EMPIREUTILITY_API static EVENT::PARAMETER EVENT::parameter_from_unistring(const UniString& parameter)
				{
								if (parameter == L"none")                                                                                            return P_NONE;
								if (parameter == L"battle_alliance")                                         return P_BATTLE_ALLIANCE;
								if (parameter == L"battle_unit")                                                 return P_BATTLE_UNIT;
								if (parameter == L"building_level")                                                          return P_BUILDING_LEVEL;
								if (parameter == L"character")                                                                    return P_CHARACTER;
								if (parameter == L"faction")                                                                         return P_FACTION;
								if (parameter == L"fort")                                                                                               return P_FORT;
								if (parameter == L"garrison_residence")                                return P_GARRISON_RESIDENCE;
								if (parameter == L"historical_character")                               return P_HISTORICAL_CHARACTER;
								if (parameter == L"historical_event")                                      return P_HISTORICAL_EVENT;
								if (parameter == L"military_force")                                                          return P_MILITARY_FORCE;
								if (parameter == L"mission")                                                                       return P_MISSION;
								if (parameter == L"mission_result")                                                         return P_MISSION_RESULT;
								if (parameter == L"port")                                                                                              return P_PORT;
								if (parameter == L"region")                                                                                          return P_REGION;
								if (parameter == L"region_slot")                                                return P_REGION_SLOT;
								if (parameter == L"settlement")                                                                return P_SETTLEMENT;
								if (parameter == L"target_character")                                     return P_TARGET_CHARACTER;
								if (parameter == L"target_faction")                                                          return P_TARGET_FACTION;
								if (parameter == L"technology")                                                                return P_TECHNOLOGY;
								if (parameter == L"unit")                                                                                               return P_UNIT;
								if (parameter == L"map_position")                                                           return P_MAP_POSITION;
								if (parameter == L"faction_sea_trade_raided")   return P_FACTION_SEA_TRADE_RAIDED;
								if (parameter == L"faction_land_trade_raided") return P_FACTION_LAND_TRADE_RAIDED;
								if (parameter == L"province")                                                                     return P_PROVINCE;

								CA_ERROR1(_U("EVENT::parameter_from_unistring: %s is not an event parameter\n"), parameter.get_temporary_uni_chars());
								return P_NUM_PARAMETERS;
				}

				void EVENT::trigger_event(const EVENT& ev)
				{
								if( !ev.event_valid_for_trigger() )
								{
												CA_ERROR("Event cannot be triggered, see Scott" );
												return;
								}

#ifndef FINAL_RELEASE
								EVENT_TRAIT_CONDITION_LOGGING::log_event(ev.event_name());
#endif
								if (m_lua_envs.size() > 0 && m_lua_envs.back() && ev.valid())
								{
												int32 successes = m_lua_envs.back()->check_event(ev);
												ev.log(successes);
								}

								// if we're in an autotest, also trigger the event to the autotest lua environment
								if (EMPIRE_LUA_ENV::s_autorun_scripting_env)
								{
												EMPIRE_LUA_ENV::s_autorun_scripting_env->check_event(ev);
								}
				}

								EMPIREUTILITY_API EVENT() = default;
								EMPIREUTILITY_API virtual ~EVENT() = default;


				private:
								String                                                                                                                                                                                                    m_event;
								static CONST_SAFE_PTR_VECTOR<EMPIRE_LUA_ENV>                                   m_lua_envs;

				private:
								#if defined LOGGING_AVAILABLE
				void EVENT::log(int32 successes) const
#else
				void EVENT::log(int32 successes) const
#endif
				{
#if defined LOGGING_AVAILABLE
								if (LOGGING::enabled())
								{
												EVENT_LOGGER logger;
												init_log(logger);

												if (logger.table_name.length() > 0)
												{
																LOG log(logger.table_name, UniString::format("trigger='%s' AND %S", event_name(), logger.key_search));
																if (log)
																{
																				log << std::make_pair("trigger", UniString(event_name()));
																				log("count") += 1;
																				log("successes") += successes;
																				for (CA_STD::VECTOR<EVENT_LOGGER::FIELD_VALUE_PAIR>::iterator it = logger.values.begin(); it != logger.values.end(); ++it)
																				{
																								log << std::make_pair((*it).field, (*it).value);
																				}
																}
												}
								}
#endif
#if defined LOG_EVENTS_TO_CONSOLE_WINDOW && defined USE_WINDOW_CONSOLE
								CONSOLE_MSG(event_tab, event_name() << endl)
#endif
				}
};


*/

//Example: A `ISubscriber` (subscriber/reporter) may even observe multiple `Subject`s (listeners) of different types:
/*
class EnvironmentPressureWindow
	: public ISubscriber<Temperature>,	// Yes Observers work with CRTP
	public ISubscriber<Pressure>
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
