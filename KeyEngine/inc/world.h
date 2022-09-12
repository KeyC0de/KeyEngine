#pragma once


class World
{
                                friend EmpireFileOutSection & operator << (EmpireFileOutSection&, const WORLD&);
                public:
                                WORLD(CAMPAIGN_ENV_MODEL_ACCESS & access, const CAMPAIGN_SETUP &setup, const EMPIREUTILITY::CAMPAIGN_RECORD&, CONST_SAFE_PTR<WORLD>& env_fixup);
                                WORLD(CAMPAIGN_ENV_MODEL_ACCESS & access, EmpireFileInSection&, CONST_SAFE_PTR<WORLD>& env_fixup, bool & error_flag);

                                virtual ~WORLD();

                                void                                                                                                                                                                       apply_changes_to_loaded_startpos(PP_ANTI_PIRACY_V2_PERFORMANCE_CHECK_PARAMETER(const CAMPAIGN_SETUP &), const FACTION_STARTING_GENERAL_OPTIONS_MAP & faction_starting_general_options_map);
                                void                                                                                                                                                                       perform_new_campaign_line_of_sight_update();
                                void                                                                                                                                                                       override_setup(const CAMPAIGN_SETUP&); // For altering games on load - ie save games as startpos

                                void                                                                                                                                                                       post_load_fixup(const CHARACTER_LOCKS_MAP & character_locks);                                                                                                                                                                                                                                                       // Fixup all pointers after a load operation
                                void                                                                                                                                                                       post_load_fixup_trade_resources(const CA_STD::VECTOR< card32 > & resource_remapping, card32 number_of_resources);
                                void                                                                                                                                                                       post_load_fixups_all_completed();                                                                                                          // All fixups done.  Any additional updates required can be done now.

                                void                                                                                                                                                                       populate_display_list(VIEW_ITEM_REGISTRAR&) const;
                                bool                                                                                                                                                                       is_it_anybodys_turn_yet()                                                                           const     { return m_is_it_anybodys_turn_yet; }
                                FACTION&                                                                                                                                                          whose_turn_is_it()                                                                                                                                         { return *m_current_turn_faction; }
                                const FACTION&                                                                                                                                              whose_turn_is_it()                                                                                                         const     { return *m_current_turn_faction; }
                                FACTION &                                                                                                                                                          whose_effective_turn_is_it();
                                EMPIRECAMPAIGN_API const FACTION &                                                             whose_effective_turn_is_it() const;
                                FACTION &                                                                                                                                                          which_AI_faction_should_be_processed();
                                const FACTION &                                                                                                                                              which_AI_faction_should_be_processed() const;
                                bool                                                                                                                                                                       end_turn(bool is_end_of_year);                                                                                                                                                                                                // Returns true if end of round.
                                void                                                                                                                                                                       start_of_turn();
                                void                                                                                                                                                                       start_of_round();
                                void                                                                                                                                                                       tick();

                                const REGION_MANAGER &                                                                                                        region_manager() const                     { return *m_region_manager; }
                                REGION_MANAGER &                                                                                                                    region_manager()                                                { return *m_region_manager; }
                                bool                                                                                                                                                                       region_manager_valid() const { return m_region_manager != null; }

                                const SEA_REGION_MANAGER &                                                                                              sea_region_manager() const      { return *m_sea_region_manager; }
                                SEA_REGION_MANAGER &                                                                                                          sea_region_manager()                  { return *m_sea_region_manager; }

                                const PROVINCE_MANAGER &                                                                                   province_manager() const { return *m_province_manager; }
                                PROVINCE_MANAGER &                                                                                                                               province_manager()                       { return *m_province_manager; }

                                const FACTION_ARRAY &                                                                                                              factions() const                 { return m_factions; }
                                                  FACTION_ARRAY &                                                                                                       factions()                                                             { return m_factions; }

                                FACTION*                                                                                                                                                           lookup_faction_by_key(const String& key); // Null if not present
                                EMPIRECAMPAIGN_API const FACTION*                                                lookup_faction_by_key(const String& key) const; // Null if not present

                                EMPIRECAMPAIGN_API const FACTION*                                                lookup_faction_by_faction_record(const FACTION_RECORD&) const;
                                FACTION*                                                                                                                                                           lookup_faction_by_faction_record(const FACTION_RECORD&);
                                // Note that factions can re-emerge and are never deleted.  They can be moved around in, and removed from, the faction array, but will
                                // only be deleted at game close.

                                // Rebel faction is always present; one faction controls all rebel armies
                                FACTION&                                                                                                                                                          rebel_faction() { return *m_rebel_faction; }
                                const FACTION&                                                                                                                                              rebel_faction() const { return *m_rebel_faction; }

                                // Slave faction may be present, but may be null in maps without slave faction!
                                FACTION*                                                                                                                                                           slave_faction() { return m_slave_faction; }
                                const FACTION*                                                                                                                                               slave_faction() const { return m_slave_faction; }

                                CLIMATE &                                                                                                                                                          climate();
                                const CLIMATE &                                                                                                                                              climate() const;

                                const FACTIONS_INTERACTIONS::FACTION_INTERACTIONS_ENV&             factions_interactions_env() const { return *m_factions_interactions_env; }
                                FACTIONS_INTERACTIONS::FACTION_INTERACTIONS_ENV&                         factions_interactions_env() { return *m_factions_interactions_env; }
                                
                                STORM_MANAGER &                                                                                                                                     storm_manager() { return *m_storm_manager; }
                                const STORM_MANAGER &                                                                                                         storm_manager() const { return *m_storm_manager; }

                                EMPIRECAMPAIGN_API card32                                                                                  storm_strength(const REGION_DATA & region) const;

                                GOD_MANAGER &                                                                                                                                          god_manager() { return *m_god_manager; }
                                const GOD_MANAGER &                                                                                                                              god_manager() const { return *m_god_manager; }

                                TRADING_RESOURCES_MANAGER &                                                                                       trading_resources_manager()                      { return *m_trading_resources_manager; }
                                const TRADING_RESOURCES_MANAGER &                                                           trading_resources_manager() const { return *m_trading_resources_manager; }

                                const FACTION_STRATEGIC_THREAT_SYSTEM&          strategic_threat_system() const   { return *m_faction_strategic_threat_system.get(); }
                                FACTION_STRATEGIC_THREAT_SYSTEM&                strategic_threat_system()         { return *m_faction_strategic_threat_system.get(); }

                                bool                                                                                                                                                                       position_visible_to_humans(LOGICAL_POSITION position) const; // tests against human shrouds for visibility
                                bool                                                                                                                                                                       character_visible_to_humans(const CHARACTER&) const; // tests against human shrouds for visibility, and whether theach has permission to see them

                                void                                                                                                                                                                       ancillary_remove_reference(const ANCILLARY&)               {} // Ancillaries unique to the world can only exist once, so never remove references

                                void                                                                                                                                      initialise_diplomacy();
                                void                                                                                                                                                                       force_diplomatic_known_state_for_active_relationships();
                                void                                                                                                                                      initialise_trade(bool startpos);
                                void                                                                                                                                                                       setup_start_pos_barter_agreements(const EMPIREUTILITY::CAMPAIGN_RECORD& campaign);

                                Image8888 *                                                                                                                                                      generate_region_ownership_map_for_faction(const String & fac, const String & theatre, const Pixel8888& owned_colour) const;

                                void                                                                                                                                      its_somebodies_turn_now() {m_is_it_anybodys_turn_yet = true;}

                                card32                                                                                                                                                                   number_of_human_factions_alive() const;
                                WORLD_TURN_PHASE                                                                                                                   turn_phase() const                { return m_turn_phase; }

                                void                                                                                                                                                                       find_siege_to_update(); // Advances the phase if none found
                                bool                                                                                                                                                                       find_multiturn_action_to_update(); // Returns true if none found

                                void                                                                                                                                                                       turn_phase_normal_end(); // In effect, end turn - but actually, we first may need to handle some things, like multiturn movement.  NOTE:  are there circumstances where we return to normal phase?
                                void                                                                                                                                                                       turn_phase_set_automanage();

                                void                                                                                                                                                                       character_creation_events_for_characters_generated_in_startpos() const;

                                PP_ANTI_PIRACY_V2_NEVER_INLINE void             unit_modifier_changed(PP_ANTI_PIRACY_V2_PERFORMANCE_CHECK_PARAMETER(float32 old_multiplier), float32 new_multiplier);

                                bool                                                                                                                                                                       is_highest_prestige_faction(const FACTION&) const;

                                CA_STD::VECTOR<POWER_PRESTIGE_WEALTH_RANKING>           faction_rankings() const;
                                const UniString& power_ranking(card32 ranking_level) const;
                                const UniString& prestige_ranking(card32 ranking_level) const;
                                const UniString& wealth_ranking(card32 ranking_level) const;

                                void                                                                                                                                                                       player_options_changed();

                                card32                                                                                                                                                                   total_income() const;

                                CAMPAIGN_MAP_PIECE &                                                                                                           allocate_map_piece(CHARACTER & character);
                                void                                                                                                                                                                       deallocate_map_piece(CAMPAIGN_MAP_PIECE & map_piece);

                                void                                                                                                                                                                       check_families();

#if !defined(FINAL_RELEASE)
                                void                                                                                                                                                                       validate_settlement_sprawl_extents() const;
#endif // !FINAL_RELEASE

                                void                                                                                                                     divide_realm(FACTION & faction);
                                bool                                                                                                                     realm_has_divided() const;
                                bool                                                                                                                     realm_can_divide() const;
                                bool                                                                                                                     realm_can_divide_from_fame(const FACTION & faction) const;
                                int32                                                                                                                    realm_divide_round() const;
                                int32                                                                                                                    realm_divide_rounds_since() const;
                                const FACTION *                                                                                                              world_leader() const { return m_world_leader; }
                                void                                                                                                                     declare_world_leader(FACTION & faction);

                                const TRADE_NODE *                                                                                                                     trade_node(const String & key) const;
                                TRADE_NODE *                                                                                                                                 trade_node(const String & key);
                                EMPIRECAMPAIGN_API const OWNED_CONST_SAFE_PTR_VECTOR< TRADE_NODE > &   trade_nodes() const;

                                card32                                                                                                                                                                   trade_resource_consumption(card32 number_of_regions) const;

#if !defined(FINAL_RELEASE)
                                void validate_map_pieces() const;
#endif

#if !defined FINAL_RELEASE
                                void destroy_all_armies();
                                void destroy_all_navies();
                                void destroy_all_armies_except_sea_locked_hordes();
                                void destroy_all_agents();
                                void destroy_all_armies(const String &faction);
                                void destroy_all_navies(const String &faction);
                                void destroy_all_armies_except_sea_locked_hordes(const String &faction);
                                void destroy_all_agents(const String &faction);
#endif

                                bool                                                                                                                                                                                       check_if_building_yet_constructed(const BUILDING_LEVEL_RECORD&) const;

                                const CONST_SAFE_PTR_VECTOR<FACTION>&                                                   mp_co_op_factions() const { return m_mp_co_op_factions; }
                                CONST_SAFE_PTR_VECTOR<FACTION>&                                                                                               mp_co_op_factions()                     { return m_mp_co_op_factions; }

                                const CONST_SAFE_PTR_VECTOR<FACTION>&                                                   human_factions() const { return m_human_factions; }
                                CONST_SAFE_PTR_VECTOR<FACTION>&                                                                                               human_factions()                            { return m_human_factions; }

                                EMPIRECAMPAIGN_API CHARACTER_OBSERVATION_OPTIONS                get_character_observation_options(FACTION_GROUP faction_group) const;                                                                                                                                                                                        // Returns the current character observation options for the specified faction group (e.g. enemies of the local faction)
                                EMPIRECAMPAIGN_API CHARACTER_OBSERVATION_OPTIONS                get_character_observation_options(const FACTION & faction, const FACTION & observer) const;                                                                                                // Returns the current character observation options for the specified faction as seen by the observer faction
                                void                                                                                                                                                                                       set_character_observation_options(const CHARACTER_OBSERVATION_OPTIONS & options);                                                                                                                                        // Sets the character observation options for the single faction specified in the options
                                void                                                                                                                                                                                       set_character_observation_options(FACTION_GROUP faction_group, CHARACTER_OBSERVATION_OPTIONS options);                                                              // sets the character observation options for all factions that match 'application_target' for the specified local_faction (e.g. all enemies of the local_faction)
                                void                                                                                                                                                                                       clear_character_observation_options(const FACTION & faction);                                                                                                                                                                                                                // Removes observation options from the per-faction list of options (so the faction's options are again chosen based on their relation to the observer (enemy, ally, neutral)

                                void                                                                                                                                                                                       load_is_completed_and_setup_correct(bool was_from_startpos); // Handle any bits and bobs that need the World in a complete state, including setup being corrected from startpos
                                card32                                                                                                                                                                                   number_of_regions_with_state_religion(const RELIGION_RECORD*) const;
                                void                                                                                                                                                                                       assemble_mp_and_human_factions();
                                
                                void                                                                                                                                                                                       campaign_process_setup_settlements_initial_zoe();
                                card32                                                                                                                                                                                   characters_checksum() const;    //it's impossible to checksum everything but this tries a bare minimum, to detect desyncs between players
                                card32                                                                                                                                                                                   economics_checksum() const;   //it's impossible to checksum everything but this tries a bare minimum, to detect desyncs between players

                                //family
                                FAMILY_TREE &                                                                                                                                                family_tree(){ return *m_family_tree; }
                                const FAMILY_TREE &                                                                                                                                    family_tree() const{ return *m_family_tree; }

                                const FERTILITY_LEVEL_RECORDS &                                                                                         fertility_levels() const;
                                float32                                                                                                                                                                                  global_religion_percentage(const RELIGION_RECORD & record) const;

                                card32                                                                                                                                                                                   number_of_trait_present_worldwide(const String & trait_key) const;

                                const MILITARY_FORCE_SETTLE_EXTENTS_MANAGER &                 miiltary_force_settle_extents_manager() const;

                                void                                                                                                                                                                                       rebuild_all_recruitment_permissions_caches();
                                void                                                                                                                                                                                       recruitment_technology_restrictions_changed();
                                void                                                                                                                                                                                       recruitment_event_restrictions_changed();

                                const CHARACTER *                                                                                                                                        get_character_from_start_pos_key(const String & key) const;

                                const ROGUE_ARMIES&                                                                                                                                rogue_armies() const { return m_rogue_armies; }
                                ROGUE_ARMIES&                                                                                                                                                           rogue_armies()                 { return m_rogue_armies; }

                                EMPIRECAMPAIGN_API CA_STD::VECTOR<const CHARACTER*> all_characters() const;
                                EMPIRECAMPAIGN_API const CAMPAIGN_MAP_WINDS_OF_MAGIC_STRENGTH_RECORD * get_active_campaign_map_winds_of_magic_strength_record_for_region(const REGION_DATA & region) const;
                                EMPIRECAMPAIGN_API const CAMPAIGN_STORM_RECORD *                get_active_storm_record_for_region(const REGION_DATA & region) const;
                                EMPIRECAMPAIGN_API const LINKED_RITUAL_CHAINS&                linked_ritual_chains() const { return m_linked_ritual_chains; }
                                LINKED_RITUAL_CHAINS&                                                                                                                           linked_ritual_chains() { return m_linked_ritual_chains; }

                                CIVILIZATION_LEVEL                                                                                                                                       get_civilization_level() const;
                                void                                                                                                                                                                                       set_civilization_level(CIVILIZATION_LEVEL new_level);
                private:

                                struct REGION_WINDS_OF_MAGIC
                                {
                                                REGION_WINDS_OF_MAGIC()
                                                                : m_strength_record(nullptr)
                                                                , m_locked(false)
                                                {}

                                                REGION_WINDS_OF_MAGIC( const EMPIREUTILITY::CAMPAIGN_MAP_WINDS_OF_MAGIC_STRENGTH_RECORD* strength_record, bool locked )
                                                                : m_strength_record(strength_record)
                                                                , m_locked(locked)
                                                {}

                                                const EMPIREUTILITY::CAMPAIGN_MAP_WINDS_OF_MAGIC_STRENGTH_RECORD* m_strength_record;
                                                bool m_locked;
                                };

                                typedef CA_STD::UNORDERED_MAP<const REGION_DATA *, REGION_WINDS_OF_MAGIC, HASH_REGION_DATA> REGION_DATA_WINDS_OF_MAGIC_STRENGTH_MAP;

                                void                                                                                                                                                                       init_faction_cache();
                                void                                                                                                                                                                       load_trade_nodes();

                                void                                                                                                                                                                       setup_fertility_levels();

                                void                                                                                                                                                                       notify (const REPORT_REGION_BUILDING_CONSTRUCTION_COMPLETED &msg);
                                void                                                                                                                                                                       notify(const REPORT_FACTION_ACHIEVED_VICTORY & msg);

                                CONST_SAFE_PTR<FACTION>                                                                                                     m_current_turn_faction;
                                CONST_SAFE_PTR<FACTION>                                                                                                     m_rebel_faction; // Always present, this is a container for rebel armies and regions
                                CONST_SAFE_PTR<FACTION>                                                                                                     m_slave_faction; // May be present, may be null - if present, will ALSO be present in the factions array, ie this is just a cached value
                                FACTION_ARRAY                                                                                                                                              m_factions;
                                CONST_SAFE_PTR<REGION_MANAGER>                                                                m_region_manager;
                                CONST_SAFE_PTR<PROVINCE_MANAGER>                                                          m_province_manager;
                                CONST_SAFE_PTR<SEA_REGION_MANAGER>                                                     m_sea_region_manager;
                                CONST_SAFE_PTR<STORM_MANAGER>                                                                m_storm_manager;
                                CONST_SAFE_PTR<GOD_MANAGER>                                                                                     m_god_manager;
                                CONST_SAFE_PTR<TRADING_RESOURCES_MANAGER>                   m_trading_resources_manager;
                                bool                                                                                                                                                                       m_result_declared;
                                bool                                                                                                                                                                       m_is_it_anybodys_turn_yet;
                                WORLD_TURN_PHASE                                                                                                                   m_turn_phase;

                                bool                                                                                                                                                                       m_faction_cache_initialized;
#if defined(USE_STEAM_CEG) && !defined(DISABLE_ANTI_PIRACY_V2)
                                bool                                    m_anti_piracy_v2_cracked_message_1;
#endif
                                CA_STD::UNORDERED_MAP<String, FACTION*>                 m_faction_from_key;
                                CA_STD::VECTOR<CHARACTER_OBSERVATION_OPTIONS>                m_faction_character_observation_options;
                                CHARACTER_OBSERVATION_OPTIONS                                                                   m_allies_character_observation_options;
                                CHARACTER_OBSERVATION_OPTIONS                                                                   m_enemies_character_observation_options;
                                CHARACTER_OBSERVATION_OPTIONS                                                                   m_neutrals_character_observation_options;

                                SPYING_LISTENER_NEXUS                                                                                                            m_spying_listener_nexus;

                                UTILITYLIB::EXTENDING_POOL< sizeof(CAMPAIGN_MAP_PIECE), 128 >   m_map_pieces;

                                int32                                                                                                                                                                      m_realm_divide_round;
                                const FACTION *                                                                                                                                              m_world_leader;
                                OWNED_CONST_SAFE_PTR_VECTOR< TRADE_NODE >                    m_trade_nodes;

                                CA_STD::VECTOR< std::pair< card32, card32 > >   m_trade_resource_consumption;

                                CA_STD::VECTOR<const BUILDING_LEVEL_RECORD*>     m_world_firsts_already_built; // Building levels that have been built already

                                CONST_SAFE_PTR_VECTOR<FACTION>                                                                  m_mp_co_op_factions;
                                CONST_SAFE_PTR_VECTOR<FACTION>                                                                  m_human_factions;
                                
                                FAMILY_TREE    *                                                                                                                                             m_family_tree;
                                CONST_SAFE_SCOPED_PTR<CLIMATE>                                                                  m_climate;
                                FERTILITY_LEVEL_RECORDS                                                                                                         m_fertility_levels;

                                CONST_SAFE_SCOPED_PTR<MILITARY_FORCE_SETTLE_EXTENTS_MANAGER>                m_settle_extents_manager;

                                REGION_DATA_WINDS_OF_MAGIC_STRENGTH_MAP                                     m_region_data_winds_of_magic_strength_map;
                                ROGUE_ARMIES                                                                                                                                               m_rogue_armies;
                                LINKED_RITUAL_CHAINS                                                                                                              m_linked_ritual_chains;
                                OWNED_CONST_SAFE_PTR<FACTION_STRATEGIC_THREAT_SYSTEM> m_faction_strategic_threat_system;
                                OWNED_CONST_SAFE_PTR<FACTIONS_INTERACTIONS::FACTION_INTERACTIONS_ENV> m_factions_interactions_env;

                                CIVILIZATION_LEVEL                                                                                                                       m_civilization_level;


                                WARSCAPE::WS_SCENE*                                                                                                                              m_ws_scene;                // class Scene    // terrain/landscape class
}
