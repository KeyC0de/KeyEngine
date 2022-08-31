#pragma once


template<typename T>
class BASE_SCRIPT_INTERFACE
{
public:
	virtual ~BASE_SCRIPT_INTERFACE()
	{
	}

	typedef BASE_SCRIPT_INTERFACE<T>				BASE_TYPE;
	typedef typename LUNAR_WITH_ALLOCATOR<T>::TYPE	LUNAR_TYPE;
	typedef typename LUNAR_TYPE::RegType			LUNAR_REG_TYPE;

	static EMPIRECAMPAIGN_API const char className[];
	static EMPIRECAMPAIGN_API LUNAR_REG_TYPE methods[];

	virtual T & operator += ( const T & )
	{
		return *static_cast<T *>( this );
	}
	virtual T & operator -= ( const T & )
	{
		return *static_cast<T *>( this );
	}
	virtual T & operator /= ( const T & )
	{
		return *static_cast<T *>( this );
	}
	virtual T & operator *= ( const T & )
	{
		return *static_cast<T *>( this );
	}
	virtual bool operator == ( const T & )
	{
		return false;
	}

	virtual int is_null_interface( lua_State * );
};




class FACTION_SCRIPT_INTERFACE
	:
	public BASE_SCRIPT_INTERFACE<FACTION_SCRIPT_INTERFACE>,
	public OBJECT_WRAPPER_INTERFACE<FACTION_SCRIPT_INTERFACE, FACTION>
{
public:

	FACTION_SCRIPT_INTERFACE()
	{
	}
	FACTION_SCRIPT_INTERFACE( lua_State * )
	{
	}

	FACTION_SCRIPT_INTERFACE( const FACTION & faction )
		:
		OBJECT_WRAPPER_INTERFACE( faction )
	{
	}

	bool operator == ( const FACTION_SCRIPT_INTERFACE & );

	int command_queue_index( lua_State * );
	// { /* implementation */ } //

	int has_faction_leader( lua_State * );
	// { /* implementation */ } //
	
	// more lua functions here..
};




template<typename SCRIPT_INTERFACE, typename WRAPPED_TYPE>
class OBJECT_WRAPPER_INTERFACE
{
public:
	OBJECT_WRAPPER_INTERFACE()
		:
		m_wrapped_object( null )
	{
	}

	OBJECT_WRAPPER_INTERFACE( const WRAPPED_TYPE & object )
		:
		m_wrapped_object( &object )
	{
	}

	virtual ~OBJECT_WRAPPER_INTERFACE()
	{
	}

	const  WRAPPED_TYPE *		unwrap() const
	{
		return m_wrapped_object;
	}
	static SCRIPT_INTERFACE *	create_and_push_to_stack( lua_State * ls, const WRAPPED_TYPE & object );

protected:
	void						fixup( const WRAPPED_TYPE & object )
	{
		m_wrapped_object = &object;
	}
	const WRAPPED_TYPE *		m_wrapped_object;
};













class FACTION : public CAMPAIGN_ENV_MODEL_ACCESS,
	public COMMAND_QUEUE_INDEXED_OBJECT<FACTION>,
	public ANCILLARY_UNIQUENESS_MONITOR,
	public SAFER_LISTENER<REGION_FACTION_CHANGE>,
	public SAFER_REPORTER<TAX_LEVELS_SET>,
	public SAFER_LISTENER< MILITARY_FORCE_DETAILS_CHANGE >,
	public SAFER_REPORTER<FACTION_LEVEL_CHANGED>,
	public SAFER_REPORTER<WAR_COORDINATION_MARKERS_CHANGED>,
	public SAFER_LISTENER< OBJECT_DESTROYED< MILITARY_FORCE > >,
	public SAFER_LISTENER< MILITARY_FORCE_FACTION_CHANGE >,
	public SAFER_LISTENER<REPORT_DIPLOMACY_CHANGED>,
	public SAFER_REPORTER<FACTION_RECRUITED_NEW_FORCE>,
	public SAFER_LISTENER<REPORT_CLIENT_STATE_CREATED>,
	public SAFER_LISTENER<REPORT_VASSAL_CREATED>,
	public SAFER_LISTENER<REPORT_FACTION_JOINS_CONFEDERATION>,
	public SAFER_LISTENER<REPORT_SETTLEMENT_ABOUT_TO_BE_CAPTURED>,
	public SAFER_LISTENER<REPORT_SETTLEMENT_ABOUT_TO_BE_RAZED>,
	public CACHED_BONUS_VALUES_AND_SOURCED_EFFECT_LIST,
	public SCOPED_EFFECTS_PROVIDER_ACTIVE_DISTRIBUTORS, // Track/invalidate distributors using our effects when they change
	public SAFER_LISTENER<REPORT_FACTION_DEATH>,
	public SAFER_LISTENER<REPORT_RITUAL_COMPLETE>,
	public SAFER_LISTENER<REPORT_POOLED_RESOURCE_TRANSACTION>,
	public SAFER_LISTENER< CAMPAIGN_GODS_AND_FAVOR_CONFLICT_STATE_CHANGE >,
	public SAFER_LISTENER< REPORT_POOLED_RESOURCE_EFFECT_CHANGED >
{






	void no_params_required( lua_State * ls )
	{
		if ( ls )
		{
			LUA::State * state = LUA::GetScriptObject( ls );

			card32 num_lua_params = state->ArgCount();
			if ( num_lua_params > 0 )
			{
				CA_ERROR( "CAMPAIGN_SCRIPTING - %d parameters found where none expected - script will fail and cause crashes\n%s", num_lua_params, generate_lua_callstack( ls ).get_temporary_chars() );
			}
		}
	}



	int32 FACTION_SCRIPT_INTERFACE::command_queue_index( lua_State * ls )
	{
		no_params_required( ls );
		return lua_return_built_in_type<card32>( ls, m_wrapped_object->command_queue_index().m_index );
	}


	{
		no_params_required( ls );
		return lua_return_built_in_type<bool>( ls, m_wrapped_object->is_rebel_faction() == false && ( nullptr != m_wrapped_object->faction_leader() ) );
	}









