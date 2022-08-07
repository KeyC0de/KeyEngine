#include "sound_object.h"


const float32 DO_NOT_FADE = 0.0f;
const int32   SOUND_DEFAULT_VOLUME_MASTER = 100;
const int32   SOUND_DEFAULT_VOLUME_MUSIC = 100;
const int32   SOUND_DEFAULT_VOLUME_ADVISOR = 100;
const int32   SOUND_DEFAULT_VOLUME_VO = 100;
const int32   SOUND_DEFAULT_VOLUME_SFX = 100;



enum AUDIO_SPEAKER_CONFIGURATION
#endif
{
	ASC_2_POINT_0_SPEAKERS = 0,
	ASC_2_POINT_0_HEADPHONES,
	ASC_5_POINT_1,
#if defined( AK_71AUDIO )
	ASC_7_POINT_1,
#endif
	ASC_MAX
};

#ifdef STRONG_TYPING
enum AUDIO_API_TYPE : int
#else
enum AUDIO_API_TYPE
#endif
{
	AST_DETECT = 0,
	AST_XAUDIO2,
	AST_DIRECT_SOUND,
	AST_WASAPI,
	AST_MAX
};

#ifdef STRONG_TYPING
enum AUDIO_SOUND_QUALITY : int
#else
enum AUDIO_SOUND_QUALITY
#endif
{
	ASQ_HIGH = 0,
	ASQ_MEDIUM,
	ASQ_LOW,
	ASQ_MAX
};

#ifdef STRONG_TYPING
enum AUDIO_MEMORY : int
#else
enum AUDIO_MEMORY
#endif
{
	AM_SMALL = 0,
	AM_MEDIUM,
	AM_LARGE,
	AM_MAX
};



//----------------------------------------------------------------------------------------------------------------
// A SOUND_UNIQUE_ID is a wrapper for a unique id which also stores the string it was constructed for debugging purposes
// It should not be used directly, but through the use of the typedefs below
// It is templated so that the typedefs can't be assigned to one another
//----------------------------------------------------------------------------------------------------------------
template <int>
class SOUND_UNIQUE_ID
{
public:
	SOUND_UNIQUE_ID()
		: m_id( AK_INVALID_UNIQUE_ID )
	{
	}

#if defined( AUDIO_DEBUG_FEATURES )
	SOUND_UNIQUE_ID( String name )
		: m_id( name.empty() ? AK_INVALID_UNIQUE_ID : get_unique_id( name ) )
		, m_debug_name( lowercase( std::move( name ) ) )
	{
	}
	SOUND_UNIQUE_ID( const SOUND_UNIQUE_ID& id )
		: m_id( id.get_id() )
		, m_debug_name( id.get_debug_name() )
	{
	}
#else
	SOUND_UNIQUE_ID( const String& name )
		: m_id( name.empty() ? AK_INVALID_UNIQUE_ID : get_unique_id( name ) )
	{
	}
#endif

	SOUND_UNIQUE_ID( AkUniqueID id )
		: m_id( id )
	{
	}

	operator AkUniqueID() const
	{
		return m_id;
	}

	bool is_valid() const
	{
		return m_id != AK_INVALID_UNIQUE_ID;
	}

	AkUniqueID get_id() const
	{
		return m_id;
	}

#if defined( AUDIO_DEBUG_FEATURES )
	const String& get_debug_name() const
	{
		return m_debug_name;
	}
#endif

private:
	AkUniqueID m_id;

#if defined( AUDIO_DEBUG_FEATURES )
	String m_debug_name;
#endif
};

typedef SOUND_UNIQUE_ID<0> ANY_EVENT; // can be interpreted as either a SOUND_EVENT or DIALOGUE_EVENT by casting.
typedef SOUND_UNIQUE_ID<1> SOUND_EVENT;
typedef SOUND_UNIQUE_ID<2> DIALOGUE_EVENT;
typedef SOUND_UNIQUE_ID<3> SOUND_GAME_PARAMETER;
typedef SOUND_UNIQUE_ID<4> SOUND_SWITCH_GROUP;
typedef SOUND_UNIQUE_ID<5> SOUND_SWITCH;
typedef SOUND_UNIQUE_ID<6> SOUND_STATE_GROUP;
typedef SOUND_UNIQUE_ID<7> SOUND_STATE;
typedef SOUND_UNIQUE_ID<8> SOUND_AUX_BUS;

//----------------------------------------------------------------------------------------------------------------
// A DIALOGUE_NODE is a wrapper for a resolved dialogue node id which also stores the
// dialogue event name and its arguments for debugging purposes
//----------------------------------------------------------------------------------------------------------------
class DIALOGUE_NODE
{
public:
	DIALOGUE_NODE()
		: m_node_id( AK_INVALID_UNIQUE_ID )
	{
	}

#if defined( AUDIO_DEBUG_FEATURES )
	DIALOGUE_NODE( AkUniqueID node_id, String debug_name )
		: m_node_id( node_id )
		, m_debug_name( lowercase( debug_name ) )
	{
	}
#endif

	DIALOGUE_NODE( AkUniqueID node_id )
		: m_node_id( node_id )
	{
	}

	DIALOGUE_NODE( DIALOGUE_NODE&& other )
		: m_node_id( other.m_node_id )
#if defined( AUDIO_DEBUG_FEATURES )
		, m_debug_name( std::move( other.m_debug_name ) )
#endif
	{
	}

	DIALOGUE_NODE( const DIALOGUE_NODE&& other )
		: m_node_id( other.m_node_id )
#if defined( AUDIO_DEBUG_FEATURES )
		, m_debug_name( std::move( other.m_debug_name ) )
#endif
	{
	}


	DIALOGUE_NODE( const DIALOGUE_NODE& other ) = default;
	DIALOGUE_NODE& operator =( const DIALOGUE_NODE& ) = default;

	operator AkUniqueID() const
	{
		return m_node_id;
	}

	[[nodiscard]] bool is_valid() const
	{
		return m_node_id != AK_INVALID_UNIQUE_ID;
	}

#if defined( AUDIO_DEBUG_FEATURES )
	[[nodiscard]] const String& get_debug_name() const
	{
		return m_debug_name;
	}
#endif

private:
	AkUniqueID m_node_id;

#if defined( AUDIO_DEBUG_FEATURES )
	String m_debug_name;
#endif
};




struct SOUND_TRANSFORM
{
	UTILITYLIB::VECTOR_3 m_position;
	UTILITYLIB::VECTOR_3 m_direction;
	UTILITYLIB::VECTOR_3 m_up;

	SOUND_TRANSFORM( UTILITYLIB::VECTOR_3 position = UTILITYLIB::VECTOR_3_ZERO, UTILITYLIB::VECTOR_3 direction = UTILITYLIB::VECTOR_3( 1, 0, 0 ), UTILITYLIB::VECTOR_3 up = UTILITYLIB::VECTOR_3( 0, 1, 0 ) ):
		m_position( position ),
		m_direction( direction ),
		m_up( up )
	{
	};
};





class SOUND_OBJECT_DYNAMIC_BALANCE
{
	IGNORED_SPHERES_DATA m_ignored_data;

	float32 m_balance = -1.0f;

public:
	void set_ignored_sphere_of_influence( const SOUND_SPHERE_OF_INFLUENCE_HANDLE & handle );
	void set_ignored_sphere_of_influences( const CA_STD::VECTOR<SOUND_SPHERE_OF_INFLUENCE_HANDLE> & handle );
	void set_ignored_spheres_entity_id( card32 id );

	void update( SOUND_OBJECT_NORMAL & sound_object, const SOUND_OBJECT_POSITION_BASE & position );
	void update( SOUND_OBJECT_NORMAL & sound_object, const SOUND_OBJECT_POSITION_BASE & position, float32 scaled_azimuth );
	void clear( SOUND_OBJECT_NORMAL & sound_object );

	float32 get_balance() const
	{
		return m_balance;
	}
};

class SOUND_OBJECT_NORMAL : public SOUND_OBJECT
{
public:
	SOUND_OBJECT_NORMAL( SOUND_REFERENCE_BLOCK & block, SOUND_OBJECT_NAME name, card32 flags, float32 obstruction_size, card32 listener_mask );
	~SOUND_OBJECT_NORMAL();

	AkGameObjectID get_id() const override
	{
		return AkGameObjectID( this );
	}

	bool can_free() const override
	{
		return m_playing_sounds.empty() && ( get_reference_block().is_unreferenced() || has_flag( SOF_AUTO_RELEASE ) );
	};
	bool is_active() const override
	{
		return !m_playing_sounds.empty();
	}

	void set_parent( const SOUND_OBJECT & parent ) override;

	void set_listeners( AkGameObjectID* listeners, card32 num_listeners ) override;
	void reset_listeners();
	bool has_overridden_listeners() const override
	{
		return m_has_overridden_listeners;
	};

#if defined( AUDIO_DEBUG_FEATURES )
	const String & get_name() const
	{
		return m_name;
	}
	void add_debug_positions( CA_STD::VECTOR<VECTOR_3> & positions ) const;
	const CA_STD::UNORDERED_MAP< card32, AkSwitchStateID > & get_switches() const
	{
		return m_switches;
	}
	const CA_STD::UNORDERED_MAP< card32, float32 > & get_game_parameters() const
	{
		return m_game_parameters;
	}
	const CA_STD::VECTOR<AkAuxSendValue> & get_aux_bus_levels() const
	{
		return m_aux_bus_levels;
	}
	float32 get_obstruction_level() const
	{
		return m_obstruction_level;
	}
	float32 get_occlusion_level() const
	{
		return m_occlusion_level;
	}
	float32 get_attenuation_scale() const
	{
		return m_attenuation_scale;
	}

	card32 num_playing_sounds() const
	{
		return m_playing_sounds.size();
	}
	const SOUND_PLAYING_EVENT & get_playing_sound( card32 i ) const
	{
		return *m_playing_sounds[i];
	}
#endif

	SOUND_PLAYING_EVENT_HANDLE post_event( const SOUND_EVENT& sound_event, AkExternalSourceInfo * external_sources, AkUInt32 num_externals, bool enable_get_play_position ) override;
	SOUND_PLAYING_EVENT_HANDLE post_event( const SOUND_EVENT& sound_event, std::function<void( AkCallbackType, AkCallbackInfo & )> callback, AkExternalSourceInfo * external_sources, AkUInt32 num_externals, AkUInt32 callback_flags, CALLBACK_LIFETIME callback_lifetime, bool enable_get_play_position ) override;

	SOUND_PLAYING_SEQUENCE_HANDLE open_sequence( bool sample_accurate ) override;
	SOUND_PLAYING_SEQUENCE_HANDLE open_sequence( std::function<void( AkCallbackType, AkCallbackInfo & )> callback, AkUInt32 callback_flags, CALLBACK_LIFETIME callback_lifetime, bool sample_accurate ) override;

#if defined( AUDIO_DEBUG_FEATURES )
	void set_switch_override( AkSwitchGroupID group_id, AkSwitchStateID state_id ) const;
	void clear_switch_override( AkSwitchGroupID group_id );
#endif

	void set_switch( AkSwitchGroupID group_id, AkSwitchStateID state_id ) override;

#if defined( AUDIO_DEBUG_FEATURES )
	void set_rtpc_override( AkRtpcID rtpc_id, AkRtpcValue rtpc_value ) const;
	void clear_rtpc_override( AkRtpcID rtpc_id );
#endif

	void set_rtpc( AkRtpcID rtpc_id, AkRtpcValue rtpc_value, float32 fade, AkCurveInterpolation curve, bool bypass_internal_interpolation ) override;
	void reset_rtpc( AkRtpcID rtpc_id, float32 fade, AkCurveInterpolation curve, bool bypass_internal_interpolation ) override;

	void set_position( const VECTOR_3 & position ) override;
	void set_position_and_direction( const VECTOR_3& position, const VECTOR_3& direction, const VECTOR_3& up = VECTOR_3( 0.0f, 1.0f, 0.0f ) ) override;
	void set_multiple_positions( const VECTOR_3 * positions, card32 count, bool single_source ) override;
	void set_multiple_positions_and_directions( const VECTOR_3 * positions, const VECTOR_3 * directions, card32 count, bool single_source ) override;
	void set_scaled_multiple_positions( float32 distance_min, float32 distance_max, const VECTOR_3 & position, const VECTOR_3 * positions, card32 count ) override;
	void set_position_line( const VECTOR_3 * positions, card32 count ) override;
	void set_position_sphere( const VECTOR_3 & position, float32 radius ) override;

	void set_position_update_threshold_delta( float32 delta ) override
	{
		m_position_update_threshold_delta = delta;
	}
	void set_direction_update_threshold_delta( float32 delta ) override
	{
		m_direction_update_threshold_delta = delta;
	}

	void set_attenuation_scaling_factor( float32 scale ) override;

	void set_obstruction_occlusion( float32 obstruction, float32 occlusion ) override;

	void set_listener_mask( card32 listener_mask );

	//void set_output_bus_level( float32 level ) override;
	void set_aux_bus_levels( AkAuxSendValue * send_values, card32 count ) override;
	void clear_aux_bus_levels() override;

	void seek_on_event_to_time( const SOUND_EVENT& sound_event, float32 time ) override;
	void seek_on_event_to_percentage( const SOUND_EVENT& sound_event, float32 percentage ) override;

	void stop_on_event( const SOUND_EVENT& sound_event, float32 fade, AkCurveInterpolation curve ) override;
	void pause_on_event( const SOUND_EVENT& sound_event, float32 fade, AkCurveInterpolation curve ) override;
	void resume_on_event( const SOUND_EVENT& sound_event, float32 fade, AkCurveInterpolation curve ) override;

	void force_stop_sounds() override;

	VECTOR_3 get_closest_position() const;

	void set_doppler( AkRtpcID rtpc_id, const VECTOR_3 & position, const VECTOR_3 & velocity ) override;
	void set_doppler( AkRtpcID rtpc_id, const VECTOR_3 & velocity ) override;

	void set_scaled_azimuth( const VECTOR_3 & position, float32 size ) override;

	void set_ignored_sphere_of_influence( const SOUND_SPHERE_OF_INFLUENCE_HANDLE & handle ) override;
	void set_ignored_sphere_of_influences( const CA_STD::VECTOR<SOUND_SPHERE_OF_INFLUENCE_HANDLE> & handles ) override;
	void set_ignored_spheres_entity_id( card32 id ) override;

	virtual void update( float32 time_delta );

	void clear_callbacks( CALLBACK_LIFETIME callback_lifetime );

	bool has_flag( SOUND_OBJECT_FLAGS flag ) const
	{
		return ( m_flags & flag ) != 0;
	}

	float32 get_dynamic_balance() const
	{
		return m_dynamic_balance.get_balance();
	}

protected:
	bool m_has_overridden_listeners = false;

private:
	CA_STD::VECTOR<std::unique_ptr<SOUND_PLAYING_EVENT, void( * )( void* )>> m_playing_sounds;

	card32 m_flags;

	std::unique_ptr<SOUND_OBJECT_POSITION_BASE, void( * )( void* )> m_position;

	SOUND_OBJECT_DYNAMIC_BALANCE m_dynamic_balance;

	std::unique_ptr<SOUND_OBJECT_DOPPLER, void( * )( void* )> m_doppler;
	std::unique_ptr<SOUND_OBJECT_SCALED_AZIMUTH, void( * )( void* )> m_scaled_azimuth;

	float32 m_environment_size;
	std::unique_ptr<SOUND_OBJECT_ENVIRONMENT, void( * )( void* )> m_environment;
	float32 m_position_update_threshold_delta;
	float32 m_direction_update_threshold_delta;

#if defined( AUDIO_DEBUG_FEATURES )
	String m_name;
	CA_STD::UNORDERED_MAP<card32, AkSwitchStateID> m_switches;
	CA_STD::UNORDERED_MAP<card32, float32> m_game_parameters;
	CA_STD::VECTOR<AkAuxSendValue> m_aux_bus_levels;
	float32 m_obstruction_level = 0.0f;
	float32 m_occlusion_level = 0.0f;
	float32 m_attenuation_scale = 1.0f;
#endif

	SOUND_OBJECT_NORMAL( const SOUND_OBJECT_NORMAL & );
	SOUND_OBJECT_NORMAL & operator=( const SOUND_OBJECT_NORMAL & );
};



void SOUND_OBJECT_NORMAL::set_switch_override( AkSwitchGroupID group_id, AkSwitchStateID state_id ) const
{
	if ( m_switches.count( group_id ) > 0 )
		AK::SoundEngine::SetSwitch( group_id, state_id, get_id() );
}

void SOUND_OBJECT_NORMAL::clear_switch_override( AkSwitchGroupID group_id )
{
	auto it = m_switches.find( group_id );
	if ( it != m_switches.end() )
		AK::SoundEngine::SetSwitch( group_id, it->second, get_id() );
}
#endif

void SOUND_OBJECT_NORMAL::set_switch( AkSwitchGroupID group_id, AkSwitchStateID state_id )
{
#if defined( AUDIO_DEBUG_FEATURES )
	if ( g_wwise_engine->is_switch_overriden( group_id ) )
	{
		if ( m_switches.count( group_id ) == 0 )
			AK::SoundEngine::SetSwitch( group_id, g_wwise_engine->get_switch_override( group_id, state_id ), get_id() );

		m_switches[group_id] = state_id;
		return;
	}

	m_switches[group_id] = state_id;
#endif
	AK::SoundEngine::SetSwitch( group_id, state_id, get_id() );
}

#if defined( AUDIO_DEBUG_FEATURES )
void SOUND_OBJECT_NORMAL::set_rtpc_override( AkRtpcID rtpc_id, AkRtpcValue rtpc_value ) const
{
	if ( m_game_parameters.count( rtpc_id ) > 0 )
		AK::SoundEngine::SetRTPCValue( rtpc_id, rtpc_value, get_id() );
}

void SOUND_OBJECT_NORMAL::clear_rtpc_override( AkRtpcID rtpc_id )
{
	auto it = m_game_parameters.find( rtpc_id );
	if ( it != m_game_parameters.end() )
		AK::SoundEngine::SetRTPCValue( rtpc_id, it->second, get_id() );
}
#endif

void SOUND_OBJECT_NORMAL::set_rtpc( AkRtpcID rtpc_id, AkRtpcValue rtpc_value, float32 fade, AkCurveInterpolation curve, bool bypass_internal_interpolation )
{
#if defined( AUDIO_DEBUG_FEATURES )
	if ( rtpc_id == g_wwise_engine->get_rtpc_override_id() )
	{
		if ( m_game_parameters.count( rtpc_id ) == 0 )
			AK::SoundEngine::SetRTPCValue( rtpc_id, g_wwise_engine->get_rtpc_override_value(), get_id() );

		m_game_parameters[rtpc_id] = rtpc_value;
		return;
	}

	m_game_parameters[rtpc_id] = rtpc_value;
#endif
	AK::SoundEngine::SetRTPCValue( rtpc_id, rtpc_value, get_id(), seconds_to_ms( fade ), curve, bypass_internal_interpolation );
}

void SOUND_OBJECT_NORMAL::reset_rtpc( AkRtpcID rtpc_id, float32 fade, AkCurveInterpolation curve, bool bypass_internal_interpolation )
{
#if defined( AUDIO_DEBUG_FEATURES )
	m_game_parameters.erase( rtpc_id );
#endif
	AK::SoundEngine::ResetRTPCValue( rtpc_id, get_id(), seconds_to_ms( fade ), curve, bypass_internal_interpolation );
}

void SOUND_OBJECT_NORMAL::set_position( const VECTOR_3 & position )
{
#if defined( AUDIO_DEBUG_FEATURES )
	if ( g_wwise_engine->debug_enabled( SOUND_DEBUG::FORCE_REVERB ) )
	{
		m_flags |= SOF_REVERB;
	}
	if ( g_wwise_engine->debug_enabled( SOUND_DEBUG::FORCE_OBSTRUCTION ) )
	{
		m_flags |= SOF_OBSTRUCTION;
	}
#endif
	if ( !m_position || m_position->get_type() != SOPT_P_SINGLE )
	{
		if ( has_flag( SOF_SMOOTH ) )
			m_position = new_object<SOUND_OBJECT_POSITION_P_SINGLE_SMOOTH>( get_id(), position );
		else
			m_position = new_object<SOUND_OBJECT_POSITION_P_SINGLE>( get_id(), position );

		if ( m_position && ( has_flag( SOF_REVERB ) || has_flag( SOF_OBSTRUCTION ) ) )
			m_environment = new_object<SOUND_OBJECT_ENVIRONMENT>( *m_position, has_flag( SOF_REVERB ), has_flag( SOF_OBSTRUCTION ), m_environment_size );
		else
			m_environment.reset();
	}
	else
		m_position->set_position( position, m_position_update_threshold_delta );
}

void SOUND_OBJECT_NORMAL::set_position_and_direction( const VECTOR_3& position, const VECTOR_3& direction, const VECTOR_3& up )
{
	if ( !m_position || m_position->get_type() != SOPT_PD_SINGLE )
	{
		if ( has_flag( SOF_SMOOTH ) )
			m_position = new_object<SOUND_OBJECT_POSITION_PD_SINGLE_SMOOTH>( get_id(), position, direction );
		else
			m_position = new_object<SOUND_OBJECT_POSITION_PD_SINGLE>( get_id(), position, direction, up );

		if ( m_position && ( has_flag( SOF_REVERB ) || has_flag( SOF_OBSTRUCTION ) ) )
			m_environment = new_object<SOUND_OBJECT_ENVIRONMENT>( *m_position, has_flag( SOF_REVERB ), has_flag( SOF_OBSTRUCTION ), m_environment_size );
		else
			m_environment.reset();
	}
	else
		m_position->set_position_and_direction( position, direction, up, m_position_update_threshold_delta, m_direction_update_threshold_delta );
}

void SOUND_OBJECT_NORMAL::set_multiple_positions( const VECTOR_3* positions, card32 count, bool single_source )
{
	if ( !m_position || m_position->get_type() != SOPT_P_MULTIPLE )
	{
		m_position = new_object<SOUND_OBJECT_POSITION_P_MULTIPLE>( get_id(), positions, count, single_source, m_position_update_threshold_delta );

		if ( m_position && ( has_flag( SOF_REVERB ) || has_flag( SOF_OBSTRUCTION ) ) )
			m_environment = new_object<SOUND_OBJECT_ENVIRONMENT>( *m_position, has_flag( SOF_REVERB ), has_flag( SOF_OBSTRUCTION ), m_environment_size );
		else
			m_environment.reset();
	}
	else
		m_position->set_multiple_positions( positions, count, single_source, m_position_update_threshold_delta );
}

void SOUND_OBJECT_NORMAL::set_multiple_positions_and_directions( const VECTOR_3* positions, const VECTOR_3* directions, card32 count, bool single_source )
{
	if ( !m_position || m_position->get_type() != SOPT_PD_MULTIPLE )
	{
		m_position = new_object<SOUND_OBJECT_POSITION_PD_MULTIPLE>( get_id(), positions, directions, count, single_source, m_position_update_threshold_delta, m_direction_update_threshold_delta );

		if ( m_position && ( has_flag( SOF_REVERB ) || has_flag( SOF_OBSTRUCTION ) ) )
			m_environment = new_object<SOUND_OBJECT_ENVIRONMENT>( *m_position, has_flag( SOF_REVERB ), has_flag( SOF_OBSTRUCTION ), m_environment_size );
		else
			m_environment.reset();
	}
	else
		m_position->set_multiple_positions_and_directions( positions, directions, count, single_source, m_position_update_threshold_delta, m_direction_update_threshold_delta );
}

void SOUND_OBJECT_NORMAL::set_scaled_multiple_positions( float32 distance_min, float32 distance_max, const VECTOR_3& position, const VECTOR_3* positions, card32 count )
{
	if ( !m_position || m_position->get_type() != SOPT_P_SCALED_MULTIPLE )
	{
		m_position = new_object<SOUND_OBJECT_POSITION_P_SCALED_MULTIPLE>( get_id(), distance_min, distance_max, position, positions, count, m_position_update_threshold_delta );

		if ( m_position && ( has_flag( SOF_REVERB ) || has_flag( SOF_OBSTRUCTION ) ) )
			m_environment = new_object<SOUND_OBJECT_ENVIRONMENT>( *m_position, has_flag( SOF_REVERB ), has_flag( SOF_OBSTRUCTION ), m_environment_size );
		else
			m_environment.reset();
	}
	else
		m_position->set_scaled_multiple_positions( distance_min, distance_max, position, positions, count, m_position_update_threshold_delta );
}

void SOUND_OBJECT_NORMAL::set_position_line( const VECTOR_3* positions, card32 count )
{
	if ( count == 0 )
	{
		m_position.reset();
		m_environment.reset();
	}
	else if ( !m_position || m_position->get_type() != SOPT_P_LINE )
	{
		m_position = new_object<SOUND_OBJECT_POSITION_LINE>( get_id(), positions, count, m_position_update_threshold_delta );

		if ( m_position && ( has_flag( SOF_REVERB ) || has_flag( SOF_OBSTRUCTION ) ) )
			m_environment = new_object<SOUND_OBJECT_ENVIRONMENT>( *m_position, has_flag( SOF_REVERB ), has_flag( SOF_OBSTRUCTION ), m_environment_size );
		else
			m_environment.reset();
	}
	else
		m_position->set_position_line( positions, count );
}

void SOUND_OBJECT_NORMAL::set_position_sphere( const VECTOR_3& position, float32 radius )
{
	if ( !m_position || m_position->get_type() != SOPT_P_SPHERE )
	{
		if ( has_flag( SOF_SMOOTH ) )
			m_position = new_object<SOUND_OBJECT_POSITION_SPHERE_SMOOTH>( get_id(), position, radius, m_position_update_threshold_delta );
		else
			m_position = new_object<SOUND_OBJECT_POSITION_SPHERE>( get_id(), position, radius, m_position_update_threshold_delta );

		if ( m_position && ( has_flag( SOF_REVERB ) || has_flag( SOF_OBSTRUCTION ) ) )
			m_environment = new_object<SOUND_OBJECT_ENVIRONMENT>( *m_position, has_flag( SOF_REVERB ), has_flag( SOF_OBSTRUCTION ), m_environment_size );
		else
			m_environment.reset();
	}
	else
		m_position->set_position_sphere( position, radius );
}

void SOUND_OBJECT_NORMAL::set_attenuation_scaling_factor( float32 scale )
{
#if defined( AUDIO_DEBUG_FEATURES )
	m_attenuation_scale = scale;
#endif
	// WWISE UPDATE
	//AK::SoundEngine::SetAttenuationScalingFactor(get_id(), scale);
	AK::SoundEngine::SetScalingFactor( get_id(), scale );
}

void SOUND_OBJECT_NORMAL::set_obstruction_occlusion( float32 obstruction, float32 occlusion )
{
	AK::SoundEngine::SetObjectObstructionAndOcclusion( get_id(), g_wwise_engine->get_listener_id(), obstruction, occlusion );

#if defined( AUDIO_DEBUG_FEATURES )
	m_obstruction_level = obstruction;
	m_occlusion_level = occlusion;
#endif
}

void SOUND_OBJECT_NORMAL::set_listener_mask( card32 listener_mask )
{
	listener_mask;
	// WWISE UPDATE
	//AK::SoundEngine::SetActiveListeners( get_id(), listener_mask );
}

//void SOUND_OBJECT_NORMAL::set_output_bus_level( float32 /*level*/ )
//{
//	WWISE UPDATE
//	AK::SoundEngine::SetGameObjectOutputBusVolume( get_id(), level );
//}

void SOUND_OBJECT_NORMAL::set_aux_bus_levels( AkAuxSendValue * send_values, card32 count )
{
#if defined( AUDIO_DEBUG_FEATURES )
	m_aux_bus_levels.assign( send_values, send_values + count );
#endif
	AK::SoundEngine::SetGameObjectAuxSendValues( get_id(), send_values, count );
}

void SOUND_OBJECT_NORMAL::clear_aux_bus_levels()
{
#if defined( AUDIO_DEBUG_FEATURES )
	m_aux_bus_levels.clear();
#endif
	AK::SoundEngine::SetGameObjectAuxSendValues( get_id(), nullptr, 0 );
}

void SOUND_OBJECT_NORMAL::seek_on_event_to_time( const SOUND_EVENT& sound_event, float32 time )
{
	AK::SoundEngine::SeekOnEvent( sound_event, get_id(), seconds_to_ms( time ) );
}

void SOUND_OBJECT_NORMAL::seek_on_event_to_percentage( const SOUND_EVENT& sound_event, float32 percentage )
{
	AK::SoundEngine::SeekOnEvent( sound_event, get_id(), percentage );
}

void SOUND_OBJECT_NORMAL::stop_on_event( const SOUND_EVENT& sound_event, const float32 fade, const AkCurveInterpolation curve )
{
	AK::SoundEngine::ExecuteActionOnEvent( sound_event, AK::SoundEngine::AkActionOnEventType_Stop, get_id(), seconds_to_ms( fade ), curve );
}

void SOUND_OBJECT_NORMAL::pause_on_event( const SOUND_EVENT& sound_event, float32 fade, AkCurveInterpolation curve )
{
	AK::SoundEngine::ExecuteActionOnEvent( sound_event, AK::SoundEngine::AkActionOnEventType_Pause, get_id(), seconds_to_ms( fade ), curve );
}

void SOUND_OBJECT_NORMAL::resume_on_event( const SOUND_EVENT& sound_event, float32 fade, AkCurveInterpolation curve )
{
	AK::SoundEngine::ExecuteActionOnEvent( sound_event, AK::SoundEngine::AkActionOnEventType_Resume, get_id(), seconds_to_ms( fade ), curve );
}

void SOUND_OBJECT_NORMAL::force_stop_sounds()
{
	AK::SoundEngine::StopAll( get_id() );
}

VECTOR_3 SOUND_OBJECT_NORMAL::get_closest_position() const
{
	VECTOR_3 return_position = VECTOR_3_ZERO;

	if ( m_position )
		m_position->get_environment_position( return_position );

	return return_position;
}

void SOUND_OBJECT_NORMAL::set_doppler( AkRtpcID rtpc_id, const VECTOR_3 & position, const VECTOR_3 & velocity )
{
	if ( !m_doppler )
		m_doppler = new_object<SOUND_OBJECT_DOPPLER>( *this, rtpc_id, position, velocity );
	else
		m_doppler->set_values( rtpc_id, position, velocity );
}

void SOUND_OBJECT_NORMAL::set_doppler( AkRtpcID rtpc_id, const VECTOR_3 & velocity )
{
	if ( !m_doppler )
		m_doppler = new_object<SOUND_OBJECT_DOPPLER>( *this, rtpc_id, velocity );
	else
		m_doppler->set_values( rtpc_id, velocity );
}

void SOUND_OBJECT_NORMAL::set_scaled_azimuth( const VECTOR_3 & position, float32 size )
{
	if ( !m_scaled_azimuth )
		m_scaled_azimuth = new_object<SOUND_OBJECT_SCALED_AZIMUTH>( position, size );
	else
		m_scaled_azimuth->set_values( position, size );
}

void SOUND_OBJECT_NORMAL::set_ignored_sphere_of_influence( const SOUND_SPHERE_OF_INFLUENCE_HANDLE & handle )
{
	m_dynamic_balance.set_ignored_sphere_of_influence( handle );
}

void SOUND_OBJECT_NORMAL::set_ignored_sphere_of_influences( const CA_STD::VECTOR<SOUND_SPHERE_OF_INFLUENCE_HANDLE> & handles )
{
	m_dynamic_balance.set_ignored_sphere_of_influences( handles );
}

void SOUND_OBJECT_NORMAL::set_ignored_spheres_entity_id( card32 id )
{
	m_dynamic_balance.set_ignored_spheres_entity_id( id );
}

void SOUND_OBJECT_NORMAL::update( float32 time_delta )
{
	if ( m_position )
	{
		m_position->update( time_delta, m_position_update_threshold_delta, m_direction_update_threshold_delta );

		if ( m_scaled_azimuth )
			m_dynamic_balance.update( *this, *m_position, m_scaled_azimuth->get_azimuth() );
		else
			m_dynamic_balance.update( *this, *m_position );
	}
	else
		m_dynamic_balance.clear( *this );

	if ( m_doppler )
		m_doppler->update( *this );

	if ( m_environment )
		m_environment->update( *this, time_delta );

	for ( size_t i = 0; i < m_playing_sounds.size(); )
	{
		if ( !m_playing_sounds[i]->is_active() )
		{
			std::swap( m_playing_sounds[i], m_playing_sounds.back() );
			m_playing_sounds.pop_back();
		}
		else
			++i;
	}
}

void SOUND_OBJECT_NORMAL::clear_callbacks( CALLBACK_LIFETIME callback_lifetime )
{
	for ( auto& i : m_playing_sounds )
	{
		i->clear_callbacks( callback_lifetime );
	}
}



class SOUND_OBJECT_DOPPLER
{
	AkRtpcID m_rtpc_id;
	VECTOR_3 m_position;
	VECTOR_3 m_velocity;
	bool m_use_positioning;

	float32 m_last_value = -1.0f;

public:
	SOUND_OBJECT_DOPPLER( SOUND_OBJECT_NORMAL & sound_object, AkRtpcID rtpc_id, const VECTOR_3 & position, const VECTOR_3 & velocity )
		: m_rtpc_id( rtpc_id )
		, m_position( position )
		, m_velocity( velocity )
		, m_use_positioning( false )
	{
		update( sound_object );
	}

	SOUND_OBJECT_DOPPLER( SOUND_OBJECT_NORMAL & sound_object, AkRtpcID rtpc_id, const VECTOR_3 & velocity )
		: m_rtpc_id( rtpc_id )
		, m_position( VECTOR_3_ZERO )
		, m_velocity( velocity )
		, m_use_positioning( true )
	{
		update( sound_object );
	}

	void set_values( AkRtpcID rtpc_id, const VECTOR_3 & position, const VECTOR_3 & velocity )
	{
		m_rtpc_id = rtpc_id;
		m_position = position;
		m_velocity = velocity;
		m_use_positioning = false;
	}

	void set_values( AkRtpcID rtpc_id, const VECTOR_3 & velocity )
	{
		m_rtpc_id = rtpc_id;
		m_position = VECTOR_3_ZERO;
		m_velocity = velocity;
		m_use_positioning = true;
	}

	void update( SOUND_OBJECT_NORMAL & sound_object )
	{
		if ( m_use_positioning )
			m_position = sound_object.get_closest_position();

		const VECTOR_3 emitter_to_listener( g_wwise_engine->get_listener_pos() - m_position );
		const float32 emitter_to_listener_mag = emitter_to_listener.modulus_fast();

		float32 new_value = 1.0f;

		if ( emitter_to_listener_mag > 0.0001f )
		{
			const float32 SS = 343.3f;

			const float32 vls = ca_min( emitter_to_listener.dot( g_wwise_engine->get_listener_velocity() ) / emitter_to_listener_mag, SS );
			const float32 vss = ca_min( emitter_to_listener.dot( m_velocity ) / emitter_to_listener_mag, SS );

			new_value = ( SS - vls ) / ( SS - vss );
		}

		if ( ca_abs( m_last_value - new_value ) > SOUND_OBJECT_MIN_DOPPLER_DELTA )
		{
			m_last_value = new_value;
			sound_object.set_rtpc( m_rtpc_id, new_value, 0.0f, AkCurveInterpolation_Linear, false );
		}
	}
};