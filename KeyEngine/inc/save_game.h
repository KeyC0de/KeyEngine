#pragma once

namespace EMPIRECAMPAIGN
{
                // forward declarations ////////////////////////////
                class CAMPAIGN_MODEL;
                class CAMPAIGN_SETUP_LOCAL;

                enum class EMPIREUTILITY::CAMPAIGN_DIFFICULTY_LEVEL : card32;

                class GAME_PERSISTENT_SESSION_ID
                {

                                DECLARE_VERSIONED;

                public:

                                EMPIRECAMPAIGN_API GAME_PERSISTENT_SESSION_ID();
                                GAME_PERSISTENT_SESSION_ID(const CA_STD::VECTOR<card64>& player_ids, const CA::TimeDate& campaign_start_timestamp);
                                GAME_PERSISTENT_SESSION_ID(EMPIREUTILITY::EmpireFileInSection&);

                                EMPIRECAMPAIGN_API const CA_STD::VECTOR<card64>& player_ids() const;
                                EMPIRECAMPAIGN_API const CA::TimeDate& campaign_start_timestamp() const;

                private:

                                CA_STD::VECTOR<card64>                                                                          m_player_ids;
                                CA::TimeDate                                                                                                    m_campaign_start_timestamp;

                };

                EMPIREUTILITY::EmpireFileOutSection& operator << (EMPIREUTILITY::EmpireFileOutSection&, const GAME_PERSISTENT_SESSION_ID&);

                struct SAVE_GAME_HEADER_POOLED_RESOURCE
                {
                                String m_resource_record_key = String::empty_string();
                                int32 m_amount = 0;
                                int32 m_income_amount = 0;
                };

                typedef CA_STD::VECTOR<String> SAVE_GAME_HEADER_FACTIONS_FOR_DIPLOMACY;
                struct SAVE_GAME_HEADER_DIPLOMACY_RELATIONS_FOR_RELATION_TYPE
                {
                                int32 m_relation_type;
                                SAVE_GAME_HEADER_FACTIONS_FOR_DIPLOMACY m_faction_keys;
                };
                typedef CA_STD::VECTOR<SAVE_GAME_HEADER_DIPLOMACY_RELATIONS_FOR_RELATION_TYPE> SAVE_GAME_HEADER_DIPLOMACY_RELATIONS;

                // Class to show extra information about the game save that can be viewed in the file requester with minimal loading
                struct SAVE_GAME_HEADER : public NON_COPYABLE
                {

                                DECLARE_VERSIONED    //lint !e1511

                public:

                                typedef CA_STD::UNORDERED_MAP<String, std::unique_ptr<Image8888>> THEATRE_IMAGE_MAP;

                                EMPIRECAMPAIGN_API SAVE_GAME_HEADER(const CAMPAIGN_MODEL&, const CAMPAIGN_SETUP_LOCAL&, const CA_STD::VECTOR<card32>& dlc_mask_history, bool autorun, const GAME_PERSISTENT_SESSION_ID& game_persistent_session_id, const CA::TimeDate& save_timestamp, const UniString& user_provided_filename, card32 dlc_mask_needed_for_this_campaign, bool is_header_created_for_actual_saving, bool is_start_turn_save);
                                SAVE_GAME_HEADER(EMPIREUTILITY::EmpireFileInSection&, const EMPIREUTILITY::EMPIRE_DATABASES&, bool read_theatre_maps);
                                SAVE_GAME_HEADER();

                                SAVE_GAME_HEADER(SAVE_GAME_HEADER&& rhs);
                                SAVE_GAME_HEADER& operator=(SAVE_GAME_HEADER&& rhs);

                                String                                                                                    m_faction_key;
                                String                                                                                    m_political_party_key;
                                String                                                                                    m_flag_path;
                                String                                                                                    m_leader_portrait;
                                card32                                                                                   m_turn;
                                card32                                                                                   m_year;
                                UniString                                                                             m_season;
                                THEATRE_IMAGE_MAP                                 m_theatre_maps;
                                CAMPAIGN_DATE                                                            m_date;
                                bool                                                                                       m_has_prebuilt_theatre_maps;
                                CA_STD::VECTOR<String>                            m_region_ownership_keys;
                                CA_STD::VECTOR<card32>                          m_region_ownership_palette_entries;
                                CA_STD::VECTOR<VECTOR_2>   m_force_locations;
                                CA_STD::VECTOR<card32>                          m_dlc_mask_history;
                                GAME_PERSISTENT_SESSION_ID               m_game_persistent_session_id;
                                EMPIREUTILITY::CAMPAIGN_DIFFICULTY_LEVEL m_campaign_difficulty_level;
                                CA::TimeDate                                                    m_save_timestamp;
                                UniString                                                                             m_user_provided_filename;
                                String                                                                                    m_campaign_key;
                                card32                                                                                   m_dlc_mask_needed_for_this_campaign;
                                MOD_TRANSACTIONS                                   m_mod_history;
                                bool                                                                                       m_is_start_turn_save;
                                CA_STD::VECTOR<SAVE_GAME_HEADER_POOLED_RESOURCE> m_economy_resources;
                                SAVE_GAME_HEADER_POOLED_RESOURCE         m_legitimacy;
                                SAVE_GAME_HEADER_DIPLOMACY_RELATIONS m_diplomacy_relations;
                                CAMPAIGN_LOCALISATION                         m_capital_name;
                                card32                                                                                   m_num_provinces_owned;
                };

                EMPIREUTILITY::EmpireFileOutSection& operator << (EMPIREUTILITY::EmpireFileOutSection&, const SAVE_GAME_HEADER&);

                struct SAVE_GAME_HEADER_MULTIPLAYER : public NON_COPYABLE
                {

                                DECLARE_VERSIONED    //lint !e1511

                public:

                                typedef CA_STD::UNORDERED_MAP<String, std::unique_ptr<Image8888>> THEATRE_IMAGE_MAP;

                                EMPIRECAMPAIGN_API SAVE_GAME_HEADER_MULTIPLAYER(const CAMPAIGN_MODEL&, const EMPIRECOMMON::EMPIRE_MP_BASE&, const UniString& description, const CA_STD::VECTOR<card32>& dlc_mask_history, bool autosave, const GAME_PERSISTENT_SESSION_ID& game_persistent_session_id, const CA::TimeDate& save_timestamp, const UniString& user_provided_filename, bool is_header_created_for_actual_saving);
                                EMPIRECAMPAIGN_API SAVE_GAME_HEADER_MULTIPLAYER(EMPIREUTILITY::EmpireFileInSection&, const EMPIREUTILITY::EMPIRE_DATABASES*, bool read_theatre_maps);
                                SAVE_GAME_HEADER_MULTIPLAYER();

                                SAVE_GAME_HEADER_MULTIPLAYER(SAVE_GAME_HEADER_MULTIPLAYER&& rhs);
                                SAVE_GAME_HEADER_MULTIPLAYER& operator=(SAVE_GAME_HEADER_MULTIPLAYER&& rhs);

                                card32                                                                                                                                                   checksum() const;
                                EMPIRECAMPAIGN_API const card64                                                      player_id_for_client_order_number(card32 client_order_number) const;

                                card32                                                                                                                                                   m_application_id;
                                UniString                                                                                                                                             m_game_name;
                                UniString                                                                                                                                             m_description;
                                card32                                                                                                                                                   m_turn;
                                card32                                                                                                                                                   m_year;
                                UniString                                                                                                                                             m_season;
                                card8                                                                                                                                                     m_num_vector_players; //number of players in the following vectors
                                card8                                                                                                                                                     m_host_vector_position; //who's the host in the following vectors
                                CA_STD::VECTOR<UniString>                                                                      m_player_name;
                                CA_STD::VECTOR<String>                                                                                            m_player_faction_key;
                                CA_STD::VECTOR<String>                                                                                            m_player_political_party_key;
                                CA_STD::VECTOR<String>                                                                                            m_player_flag_path;
                                CA_STD::VECTOR<card64>                                                                                          m_player_id;
                                CA_STD::VECTOR<card8>                                                                                             m_player_resumed_campaign_difficulty_campaign;
                                CA_STD::VECTOR<card8>                                                                                             m_player_resumed_campaign_difficulty_battle;
                                THEATRE_IMAGE_MAP                                                                                                 m_theatre_maps;
                                bool                                                                                                                                                       m_autosave;
                                CAMPAIGN_DATE                                                                                                                            m_date;
                                bool                                                                                                                                                       m_has_prebuilt_theatre_maps;
                                CA_STD::VECTOR<CA_STD::VECTOR<String>>                     m_region_ownership_keys;
                                CA_STD::VECTOR<CA_STD::VECTOR<card32>>                   m_region_ownership_palette_entries;
                                CA_STD::VECTOR<CA_STD::VECTOR<VECTOR_2>>            m_force_locations;
                                CA_STD::VECTOR<card32>                                                                                          m_dlc_mask_history;
                                GAME_PERSISTENT_SESSION_ID                                                                               m_game_persistent_session_id;
                                EMPIREUTILITY::CAMPAIGN_DIFFICULTY_LEVEL                                                                 m_campaign_difficulty_level;
                                CA::TimeDate                                                                                                                    m_save_timestamp;
                                UniString                                                                                                                                             m_user_provided_filename;
                                String                                                                                                                                                    m_campaign_key;
                                CA_STD::VECTOR<String>                                                                                            m_player_leader_portraits;
                                CA_STD::VECTOR<CA_STD::VECTOR<SAVE_GAME_HEADER_POOLED_RESOURCE>>                m_player_economy_resources;
                                CA_STD::VECTOR<SAVE_GAME_HEADER_POOLED_RESOURCE>                                                                  m_player_legitimacy;
                                CA_STD::VECTOR<SAVE_GAME_HEADER_DIPLOMACY_RELATIONS>                                                         m_player_diplomacy_relations;
                                CA_STD::VECTOR<CAMPAIGN_LOCALISATION>                                                                                                                  m_player_capital_names;
                                CA_STD::VECTOR<card32>                                                                                                                                                                                          m_player_num_provinces_owned;
                                MOD_TRANSACTIONS m_mod_history;

                };

                EMPIREUTILITY::EmpireFileOutSection & operator << (EMPIREUTILITY::EmpireFileOutSection &, const SAVE_GAME_HEADER_MULTIPLAYER &);

                // public ("extern") data declarations /////////////

                struct LAST_SAVE_GAME_INFO
                {
                                bool valid = false;
                                bool from_cloud = false;
                                card32 write_time = 0;

                                UniString file_name_base;
                                UniString file_name_with_extension;
                };

                EMPIRECAMPAIGN_API LAST_SAVE_GAME_INFO FindLastSavedGame(EMPIRECOMMON::GAME_CORE &);
                EMPIRECAMPAIGN_API LAST_SAVE_GAME_INFO FindLastLoadableSavedGame(EMPIRECOMMON::GAME_CORE &);
                EMPIRECAMPAIGN_API LAST_SAVE_GAME_INFO FindLastSavedMPGame(EMPIRECOMMON::GAME_CORE &);

                EMPIRECAMPAIGN_API void cloud_save_game_sync_start(EMPIRECOMMON::EMPIRE_CLOUD_INTERFACE &cloud);
                EMPIRECAMPAIGN_API void cloud_save_game_sync_wait_for_completion();
                EMPIRECAMPAIGN_API bool cloud_save_game_sync_is_done();
                EMPIRECAMPAIGN_API std::pair<int, int> cloud_save_game_sync_progress_info(); // done_count / total_count

                // public function declarations ////////////////////
} // namespace EMPIREUTILITY
