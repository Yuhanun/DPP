#pragma once
#include <assets.hpp>
#include <discord.hpp>
#include <nlohmann/json.hpp>

namespace discord {
    
    typedef int64_t snowflake;

    class Bot {
    public:
        Bot(std::string const&, const std::string, std::size_t = 5000);
        template <size_t EVENT, typename FType>
        void register_callback(FType&& func) {
            std::get<EVENT>(func_holder.tuple).push_back(std::forward<FType>(func));
        }

        void register_command(std::string const& command_name, std::function<void(discord::Context const&)> function);
        void update_presence(Activity const&);

        pplx::task<discord::Message> send_message(snowflake, std::string, bool = false);
        pplx::task<discord::Message> send_message(snowflake, nlohmann::json, bool = false);
        pplx::task<discord::Guild> create_guild(std::string const&, std::string const& = "us-east", int const& = 0, int const& = 0, int const& = 0);

        void on_incoming_packet(const websocketpp::connection_hdl&, const client::message_ptr&);
        void handle_gateway();

        int run();

        std::vector<VoiceRegion> get_voice_regions() const;

        pplx::task<discord::User> get_current_user();
        pplx::task<discord::User> get_user(snowflake);
        // TODO: avatar
        pplx::task<discord::User> edit(std::string const&);
        pplx::task<std::vector<discord::Guild>> get_user_guilds(int = 0, snowflake = 0, snowflake = 0);

        pplx::task<discord::Channel> create_group_dm(std::vector<std::string> const&, nlohmann::json const&);
        pplx::task<std::vector<discord::Connection>> get_connections();
        pplx::task<discord::Channel> get_channel(snowflake);
        pplx::task<discord::Guild> get_guild(snowflake);

        int wait_for_ratelimits(snowflake, int);
        void handle_ratelimits(web::http::http_headers&, snowflake, int);

    private:
        void fire_commands(discord::Message&);
        void await_events();
        void gateway_auth();
        void handle_heartbeat();
        void handle_event(nlohmann::json const, std::string);
        void initialize_variables(const std::string);
        template <std::size_t event_type>
        discord::Message process_message_cache(discord::Message* m, bool&);


        void hello_event(nlohmann::json);
        void ready_event(nlohmann::json);
        void resumed_event(nlohmann::json);
        void invalid_session_event(nlohmann::json);
        void channel_create_event(nlohmann::json);
        void channel_update_event(nlohmann::json);
        void channel_delete_event(nlohmann::json);
        void channel_pins_update_event(nlohmann::json);
        void guild_create_event(nlohmann::json);
        void guild_update_event(nlohmann::json);
        void guild_delete_event(nlohmann::json);
        void guild_ban_add_event(nlohmann::json);
        void guild_ban_remove_event(nlohmann::json);
        void guild_emojis_update_event(nlohmann::json);
        void guild_integrations_update_event(nlohmann::json);
        void guild_member_add_event(nlohmann::json);
        void guild_member_remove_event(nlohmann::json);
        void guild_member_update_event(nlohmann::json);
        void guild_members_chunk_event(nlohmann::json);
        void guild_role_create_event(nlohmann::json);
        void guild_role_update_event(nlohmann::json);
        void guild_role_delete_event(nlohmann::json);
        void message_create_event(nlohmann::json);
        void message_update_event(nlohmann::json);
        void message_delete_event(nlohmann::json);
        void message_delete_bulk_event(nlohmann::json);
        void message_reaction_add_event(nlohmann::json);
        void message_reaction_remove_event(nlohmann::json);
        void message_reaction_remove_all_event(nlohmann::json);
        void presence_update_event(nlohmann::json);
        void typing_start_event(nlohmann::json);
        void user_update_event(nlohmann::json);
        void voice_state_update_event(nlohmann::json);
        void voice_server_update_event(nlohmann::json);
        void webhooks_update_event(nlohmann::json);

        std::string get_gateway_url() const;
        std::string get_identify_packet();

    public:
        bool authenticated;
        std::string error_message;
        std::string discriminator;

        snowflake id;

        bool bot;
        bool ready;
        bool verified;
        bool mfa_enabled;

        std::string token;
        std::string email;
        std::string prefix;
        std::string username;
        discord::Asset avatar;

        function_handler func_holder;
        std::vector<std::shared_ptr<discord::User>> users;
        std::vector<std::shared_ptr<discord::Guild>> guilds;
        std::vector<std::shared_ptr<discord::Channel>> channels;

    private:
        nlohmann::json hello_packet;
        nlohmann::json ready_packet;
        std::size_t message_cache_count;

        bool disconnected;
        bool heartbeat_acked;
        int last_sequence_data;
        long long packet_counter;

        std::string session_id;

        client c;
        client::connection_ptr con;

        std::thread gateway_thread;
        std::thread heartbeat_thread;

        std::vector<pplx::task<void>> futures;
        std::vector<std::shared_ptr<discord::Message>> messages;
        std::unordered_map<std::string, std::function<void(discord::Context const&)>> command_map;
        std::unordered_map<std::string, std::function<void(nlohmann::json)>> internal_event_map;

        std::unordered_map<snowflake, RateLimit> guild_ratelimits;
        std::unordered_map<snowflake, RateLimit> channel_ratelimits;
        std::unordered_map<snowflake, RateLimit> webhook_ratelimits;
        RateLimit global_ratelimits;
    };
};  // namespace discord