#pragma once
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include <boost/algorithm/string.hpp>

#include "function_type.hpp"
#include "gatewayhandler.hpp"
#include "events.hpp"

#include "cpr/cpr.h"

namespace discord {

    class Bot;
    class User;
    class Role;
    class Color;
    class Emoji;
    class Guild;
    class Member;
    class Invite;
    class Webhook;
    class Channel;
    class Message;
    class Activity;
    class EmbedBuilder;
    class PermissionOverwrite;
    class PermissionOverwrites;

    namespace detail {
        inline discord::Bot* bot_instance;
    }  // namespace detail

    typedef int64_t snowflake;
    typedef boost::posix_time::ptime datetime;


    using namespace boost;

    typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
    typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

    using websocketpp::lib::bind;

    typedef Events<
        void(),                                             // HELLO
        void(),                                             // READY
        void(),                                             // RESUMED
        void(),                                             // INVALID_SESSION,
        void(discord::Channel),                             // CHANNEL_CREATE
        void(discord::Channel),                             // CHANNEL_UPDATE
        void(discord::Channel),                             // CHANNEL_DELETE
        void(discord::Channel),                             // CHANNEL_PINS_UPDATE
        void(discord::Guild),                               // GUILD_CREATE
        void(discord::Guild),                               // GUILD_UPDATE
        void(discord::Guild),                               // GUILD_DELETE
        void(discord::Guild, discord::User),                // GUILD_BAN_ADD,
        void(discord::Guild, discord::User),                // GUILD_BAN_REMOVE,
        void(discord::Guild, std::vector<discord::Emoji>),  // GUILD_EMOJIS_UPDATE
        void(discord::Guild),                               // GUILD_INTEGRATIONS_UPDATE
        void(discord::Member),                              // GUILD_MEMBER_ADD
        void(discord::User),                                // GUILD_MEMBER_REMOVE
        void(discord::Member),                              // GUILD_MEMBER_UPDATE
        void(),                                             // GUILD_MEMBERS_CHUNK
        void(discord::Role),                                // GUILD_ROLE_CREATE
        void(discord::Role),                                // GUILD_ROLE_UPDATE
        void(discord::Role),                                // GUILD_ROLE_DELETE
        void(discord::Message),                             // MESSAGE_CREATE
        void(discord::Message),                             // MESSAGE_UPDATE
        void(discord::Message),                             // MESSAGE_DELETE
        void(std::vector<discord::Message>),                // MESSAGE_DELETE_BULK
        void(discord::Message),                             // MESSAGE_REACTION_ADD
        void(discord::Message),                             // MESSAGE_REACTION_REMOVE
        void(discord::Message),                             // MESSAGE_REACTION_REMOVE_ALL
        void(discord::User),                                // PRECENSE_UPDATE
        void(discord::Member, discord::Channel),            // PRESENCE_UPDATE
        void(discord::User),                                // USER_UPDATE
        void(discord::Member, discord::Channel),            // VOICE_STATE_UPDATE
        void(discord::Guild),                               // VOICE_SERVER_UPDATE
        void(discord::Guild)>                               // WEBHOOKS_UPDATE
        function_handler;

    using websocketpp::lib::bind;
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;

    namespace presence {
        struct status {
            inline const static std::string online = "online";
            inline const static std::string dnd = "dnd";
            inline const static std::string idle = "idle";
            inline const static std::string invisible = "invisible";
            inline const static std::string offline = "offline";
        };

        enum class activity : short {
            playing,
            streaming,
            listening,
            watching
        };
    };  // namespace presence

    class Object {
    public:
        Object() = default;

        Object(snowflake id)
            : id{ id } {
        }

        snowflake id;


        bool operator==(const Object& other) {
            return this->id == other.id;
        }

        bool operator==(const snowflake& other) {
            return this->id == other;
        }
        template <typename T>
        bool operator!=(T&& other) {
            return !(std::forward<T>(other) == this);
        }

        operator snowflake() {
            return id;
        }

        friend std::ostream& operator<<(std::ostream& stream, Object& o) {
            stream << o.id;
            return stream;
        }
    };

    struct VoiceRegion {
        std::string id;
        std::string name;
        bool vip;
        bool optimal;
        bool deprecated;
        bool custom;
    };

    class Integration {
    public:
        Integration(nlohmann::json const);

        snowflake id;
        std::string name;
        std::string type;
        bool enabled;
        bool syncing;
        std::shared_ptr<discord::Role> role;
        int expire_behavior;
        int expire_grace_period;
        std::shared_ptr<discord::User> user;
        snowflake account_id;
        std::string account_name;
        datetime synced_at;
    };

    struct Connection {
        snowflake id;
        std::string name;
        std::string type;
        bool revoked;
        std::vector<Integration> integrations;
        bool verified;
        bool friend_sync;
        bool show_activity;
        int visibility;
    };

    class Attachment {
    public:
        Attachment() = default;
        Attachment(nlohmann::json const);

        int size;
        int width;
        int height;
        snowflake id;
        std::string url;
        std::string filename;
        std::string proxy_url;
    };

    class User : public Object {
    public:
        User() = default;
        User(snowflake);
        User(nlohmann::json const);

        discord::Channel create_dm();

    public:
        bool bot;

        std::string name;

        std::string avatar;
        std::string mention;
        std::string discriminator;

    private:
        std::string get_create_dm_url();
    };

    class Member : public User {
    public:
        Member() = default;
        Member(snowflake);
        Member(nlohmann::json const, discord::User const&, discord::Guild*);

        void edit(std::string const&, bool, bool, std::vector<discord::Role> const& = {}, snowflake = -1);
        void add_role(discord::Role const&);
        void remove_role(discord::Role const&);

        void kick();
        void ban(std::string const&, int);

    public:
        bool deaf;
        bool muted;

        std::shared_ptr<discord::Guild> guild;

        std::vector<discord::Role> roles;

        std::string nick;
        datetime joined_at{ boost::local_time::not_a_date_time };
    };

    struct Context {
        Context(const discord::Bot*, discord::Message const&, std::vector<std::string> const&, std::function<void(Context const&)> const&, std::string const&);
        const discord::Bot* bot;
        const discord::Message& message;
        const std::vector<std::string>& arguments;
        const std::function<void(Context const&)> command;
        const std::string command_name;
        const std::shared_ptr<discord::Member> author;
        const std::shared_ptr<discord::Channel> channel;

        template <typename... Tys>
        discord::Message send(Tys&&... args) const;
    };


    class Bot {
    public:
        Bot(std::string const&, const std::string, std::size_t = 5000);
        template <size_t EVENT, typename FType>
        void register_callback(FType&& func) {
            std::get<EVENT>(func_holder.tuple).push_back(std::forward<FType>(func));
        }

        void register_command(std::string const& command_name, std::function<void(discord::Context const&)> function);
        void update_presence(Activity const&);

        discord::Message send_message(snowflake, std::string, bool = false);
        discord::Message send_message(snowflake, nlohmann::json, bool = false);
        discord::Guild create_guild(std::string const&, std::string const& = "us-east", int const& = 0, int const& = 0, int const& = 0);

        void on_incoming_packet(const websocketpp::connection_hdl&, const client::message_ptr&);
        void handle_gateway();

        void run();

        std::vector<VoiceRegion> get_voice_regions() const;

        discord::User get_current_user();
        discord::User get_user(snowflake);
        // TODO: avatar
        discord::User edit(std::string const&);
        std::vector<discord::Guild> get_user_guilds(int = 0, snowflake = 0, snowflake = 0);

        discord::Channel create_group_dm(std::vector<std::string> const&, nlohmann::json const&);
        std::vector<discord::Connection> get_connections();
        discord::Channel get_channel(snowflake);
        discord::Guild get_guild(snowflake);

    private:
        std::string get_current_user_url();
        std::string get_user_url(snowflake);
        std::string get_channel_url(snowflake);
        std::string get_user_guilds_url();
        std::string get_create_group_dm_url();
        std::string get_connections_url();

        void fire_commands(discord::Message&) const;
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
        std::string get_create_guild_url() const;
        std::string get_voice_regions_url() const;

        cpr::Header get_basic_header() const;

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
        std::string avatar;
        std::string prefix;
        std::string username;

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

        std::future<void> gateway_thread;
        std::future<void> heartbeat_thread;
        std::future<void> client_future;

        std::vector<discord::Message> messages;
        std::vector<std::future<void>> packet_handling;
        std::unordered_map<std::string, std::function<void(discord::Context const&)>> command_map;
        std::unordered_map<std::string, std::function<void(nlohmann::json)>> internal_event_map;
    };

    class Activity {
    public:
        Activity() = default;
        Activity(std::string const& name, presence::activity const& type, std::string const& status = "online", bool const& afk = false, std::string const& url = "");

        nlohmann::json to_json() const;

    private:
        bool afk;

        std::string url;
        std::string name;
        std::string status;

        presence::activity type;
    };

    class Channel : public Object {
    public:
        Channel() = default;
        Channel(snowflake id);

        Channel(nlohmann::json const, snowflake);

        discord::Message send(std::string const&, bool = false) const;
        discord::Message send(EmbedBuilder const&, bool = false, std::string const& = "") const;
        discord::Message get_message(snowflake);
        discord::Invite create_invite(int = 86400, int = 0, bool = false, bool = false) const;
        std::vector<discord::Invite> get_invites();
        std::vector<discord::Message> get_pins();
        void remove_permissions(discord::Object const&);
        void typing();

        std::vector<discord::Message> get_messages(int);

        void bulk_delete(std::vector<discord::Message>&);

        void edit(nlohmann::json&);
        void remove();

        // TODO: avatar
        discord::Webhook create_webhook(std::string const&);
        std::vector<discord::Webhook> get_webhooks();

        void add_group_dm_recipient(discord::User const&, std::string const&, std::string const&);
        void remove_group_dm_recipient(discord::User const&);

        void edit_position(int);

    private:
        std::string get_bulk_delete_url() const;
        std::string get_get_messages_url(int) const;
        std::string get_channel_edit_url() const;
        std::string get_delete_url() const;
        std::string get_get_message_url(snowflake) const;
        std::string get_channel_invites_url() const;
        std::string get_create_invite_url() const;
        std::string get_delete_channel_permission_url(discord::Object const&) const;
        std::string get_typing_url() const;
        std::string get_pins_url() const;

        std::string get_create_webhook_url() const;
        std::string get_webhooks_url() const;

        std::string get_add_group_dm_recipient_url(discord::User const&);

    public:
        int type;
        int bitrate;
        int position;
        int parent_id;
        int user_limit;
        int rate_limit_per_user;

        snowflake last_message_id;

        std::string name;
        std::string topic;

        std::shared_ptr<discord::Guild> guild;
        std::vector<std::shared_ptr<discord::User>> recipients;
        std::vector<discord::PermissionOverwrites> overwrites;
    };

    class Emoji : public Object {
    public:
        Emoji() = default;
        Emoji(nlohmann::json);
        operator std::string();

    public:
        bool managed;
        bool animated;
        bool require_colons;

        snowflake id;
        std::string url;
        std::string name;
        discord::User user;
        std::vector<discord::Role> roles;
    };
	struct AuditLogKeyChange {
		std::string name;
		std::string icon_hash;
		std::string splash_hash;
		snowflake owner_id;
		std::string region;
		snowflake afk_channel_id;
		int afk_timeout;
		int mfa_level;
		int verification_level;
		int explicit_content_filter;
		int default_message_notifications;
		std::string vanity_url_code;
		int prune_delete_days;
		bool widget_enabled;
		snowflake widget_channel_id;
		int position;
		std::string topic;
		int bitrate;
		bool nsfw;
		snowflake application_id;
		int permission;
		int color;
		bool hoist;
		bool mentionable;
		int allow;
		int deny;
		std::string code;
		snowflake channel_id;
		int max_uses;
		int uses;
		int max_age;
		bool temporary;
		bool deaf;
		bool mute;
		std::string nick;
		std::string avatar_hash;
		snowflake id;
		std::string type;
	};
	class AuditLogEntry {
	public:
		AuditLogEntry() = default;
		AuditLogEntry(const snowflake&, const int&, const snowflake&, const int&);
		std::string key;
		std::string new_value;
		std::string old_value;
		snowflake channel_id;
		snowflake id;
		std::string role_name;
		std::string delete_member_days;
		std::string members_removed;
		std::string count;
		std::string type;
		std::string target_id;
		snowflake user_id;
		AuditLogEvents action_type;
		std::string reason;
	};

	struct AuditLogs {
		AuditLogs() = default;
		AuditLogs(const nlohmann::json&);
	private:
		std::vector<discord::Webhook> webhooks;
		std::vector<discord::User> users;
		std::vector<discord::AuditLogEntry> audit_log_entries;
	};
    
	class Invite {
    public:
        Invite() = default;
        Invite(nlohmann::json const);
        Invite(std::string const&);

        int uses;
        int max_age;
        int max_uses;
        bool temporary;

        std::string code;
        datetime created_at{ boost::local_time::not_a_date_time };
        discord::Member inviter;
        std::shared_ptr<discord::Channel> channel;

        discord::Invite get_invite();
        void remove();

    private:
        std::string get_invite_url();
        std::string get_delete_invite_url();
    };


    class Guild : public Object {
    public:
        Guild() = default;
        Guild(snowflake);

        Guild(nlohmann::json const);

        void edit(std::string const&, std::string const& = "", int = -1, int = -1, int = -1, snowflake = -1, int = -1, std::string const& = "", snowflake = -1, std::string const& = "", snowflake = -1);
        void remove();
        std::vector<discord::Channel> get_channels();
        discord::Channel create_channel(std::string const&, bool, int = -1, std::string const& = "", int = -1, int = -1, int = -1, int = -1, std::vector<discord::PermissionOverwrites> const& = {}, snowflake = -1);

        discord::Member get_member(snowflake);
        std::vector<discord::Member> get_members(int, snowflake = 0);
        void add_member(nlohmann::json const&, snowflake);

        void edit_bot_username(std::string const&);
	    AuditLogs get_audit_logs();
        std::vector<std::pair<std::string, discord::User>> get_bans();
        std::pair<std::string, discord::User> get_ban(discord::Object const&);

        void unban(discord::Object const&);

        // TODO: Asset<>
        discord::Emoji create_emoji(std::string const&, discord::Emoji const&, std::vector<discord::Role> = {});

        std::vector<discord::Role> get_roles();

        discord::Role create_role(std::string const&, PermissionOverwrites&, discord::Color, bool, bool);

        int get_prune_count(int);
        int begin_prune(int, bool);

        std::vector<discord::VoiceRegion> get_voice_regions();
        std::vector<discord::Invite> get_invites();

        snowflake get_embed();
        snowflake edit_embed(snowflake = -1);

        std::string get_vanity_invite_url();
        // TODO: turn into an Asset<>
        std::string get_widget_image(std::string const&);
        std::vector<discord::Integration> get_integrations();
        void create_integration(discord::Integration const&);
        void edit_integration(discord::Integration const&, int, int, bool);
        void remove_integration(discord::Integration const&);
        void sync_integration(discord::Integration const&);

    public:
        int splash;
        int mfa_level;
        int afk_timeout;
        int member_count;
        int verification_level;
        int explicit_content_filter;

        bool large;
        bool unavailable;

        snowflake id;
        snowflake application_id;

        std::string name;
        std::string icon;
        std::string region;
        std::string banner;
        datetime created_at{ boost::local_time::not_a_date_time };
        std::string vanity_url_code;

        std::vector<int> features;
        std::vector<discord::Role> roles;
        std::vector<discord::Emoji> emojis;
        std::vector<std::shared_ptr<discord::Member>> members;
        std::vector<std::shared_ptr<discord::Channel>> channels;

        discord::Member owner;
        discord::Channel afk_channel;
        discord::Channel system_channel;

        std::vector<discord::Webhook> get_webhooks();
        void leave();

        std::vector<discord::Emoji> list_emojis();
        discord::Emoji get_emoji(discord::Emoji const&);
        // TODO: discord::Emoji create_emoji();
        discord::Emoji edit_emoji(discord::Emoji const&, std::string, std::vector<discord::Role> = {});
        void remove_emoji(discord::Emoji const&);

    private:
        std::string get_webhooks_url() const;
        std::string get_leave_url();

        std::string get_list_guild_emojis_url();
        std::string get_guild_emoji(snowflake);
        // TODO: std::string get_create_emoji_url();
        std::string get_modify_guild_emoji_url(snowflake);
        std::string get_delete_emoji_url(snowflake);
    };

    class Webhook : public Object {
    public:
        Webhook() = default;
        Webhook(snowflake);
        Webhook(snowflake, std::string const&);
        Webhook(nlohmann::json const);

        snowflake id;
        discord::Guild guild;
        discord::Channel channel;
        std::optional<discord::User> user;
        std::string name;
        std::string avatar;
        std::string token;

        // TODO: avatar edit for both
        void edit(std::string const& = "", snowflake = 0);
        void edit(std::string const& = "");

        void remove();

        discord::Message send(std::string const&, bool = false, std::string const& = "", std::string const& = "");
        discord::Message send(std::vector<EmbedBuilder> const&, bool = false, std::string const& = "", std::string const& = "", std::string const& = "");

        void execute_slack(bool, nlohmann::json const);
        void execute_github(bool, nlohmann::json const);

    private:
        std::string get_edit_webhook_url() const;
        std::string get_edit_webhook_token_url() const;
        std::string get_delete_webhook_url() const;
        std::string get_execute_webhook_url() const;
    };

    class Message : public Object {
    public:
        Message() = default;
        Message(snowflake);
        Message(nlohmann::json const);

        discord::Message edit(std::string);
        discord::Message edit(EmbedBuilder, std::string = "");
        void pin();
        void unpin();
        void remove();

        void add_reaction(discord::Emoji const&);
        void remove_own_reaction(discord::Emoji const&);
        void remove_reaction(discord::User const&, discord::Emoji const&);
        void remove_all_reactions();
        std::vector<std::shared_ptr<discord::User>> get_reactions(discord::Emoji const&, snowflake = 0, snowflake = 0, int = 0);

        discord::Message& update(nlohmann::json const);

    private:
        std::string get_remove_all_reactions_url();
        std::string get_remove_user_reaction_url(discord::Emoji const&, discord::User const&);
        std::string get_add_reaction_url(discord::Emoji const&) const;
        std::string get_remove_user_url(discord::User const&, discord::Emoji const&);
        std::string get_reactions_url(discord::Emoji const&);
        std::string get_edit_url() const;
        std::string get_delete_url() const;
        std::string get_pin_url() const;
        std::string get_unpin_url() const;


    public:
        int type;

        bool tts;
        bool sent;
        bool pinned;
        bool mention_everyone;

        snowflake id;

        std::string content;
        datetime timestamp{ boost::local_time::not_a_date_time };
        datetime edited_timestamp{ boost::local_time::not_a_date_time };

        std::shared_ptr<discord::Member> author;
        std::shared_ptr<discord::Channel> channel;

        std::vector<std::shared_ptr<discord::Member>> mentions;
        std::vector<discord::Role> mentioned_roles;
        std::vector<discord::Attachment> attachments;
        std::vector<discord::EmbedBuilder> embeds;

    private:
        std::string token;
    };

    class EmbedBuilder {
    public:
        EmbedBuilder(nlohmann::json);
        EmbedBuilder();
        EmbedBuilder& set_title(std::string const&);
        EmbedBuilder& set_description(std::string const&);
        EmbedBuilder& set_url(std::string const&);
        EmbedBuilder& set_timestamp(std::string const&);
        EmbedBuilder& set_color(const Color);
        EmbedBuilder& set_footer(std::string const&, std::string const&);
        EmbedBuilder& set_image(std::string const&, const int = -1, const int = -1);
        EmbedBuilder& set_thumbnail(std::string const&, const int = -1, const int = -1);
        EmbedBuilder& set_video(std::string const&, const int = -1, const int = -1);
        EmbedBuilder& set_author(std::string const&, std::string const&, std::string const&);
        EmbedBuilder& add_field(std::string const&, std::string const&, const bool = false);
        nlohmann::json to_json() const;
        operator nlohmann::json();

    private:
        nlohmann::json embed;
    };

    class Color {
    public:
        Color() = default;
        Color(int, int, int, int = -1);
        Color(int);

    public:
        int raw_int;
        int r;
        int g;
        int b;
    };

    class PermissionOverwrite {
    public:
        PermissionOverwrite() = default;
        PermissionOverwrite(int, int);
        PermissionOverwrite& add_permission(std::string const&);

        void calculate_value();
        void set_table();

        int value;
        int allow_type;

        std::unordered_map<std::string, int> ows;
    };

    class PermissionOverwrites {
    public:
        PermissionOverwrites() = default;
        PermissionOverwrites(int, int, snowflake, int);
        PermissionOverwrites(snowflake, int);

        PermissionOverwrites& add_permission(std::string const&, int);

        nlohmann::json to_json() const;
        std::pair<int, int> get_values() const;

        int object_type;

        snowflake object_id;

        PermissionOverwrite allow_perms;
        PermissionOverwrite deny_perms;
        int base_permissions;
    };

    class Role : public Object {
    public:
        Role() = default;
        Role(snowflake);
        Role(nlohmann::json, discord::Guild*);

        void edit_position(int);
        void edit(std::string const&, PermissionOverwrites&, discord::Color, bool, bool);
        void remove();

    public:
        bool hoist;
        bool managed;
        bool mentionable;
        std::shared_ptr<discord::Guild> guild;

        std::string name;

        Color color;
        PermissionOverwrites permissions;
    };

    class ImproperToken : public std::exception {
        const char* what() const throw();
    };

    class UnknownChannel : public std::exception {
        const char* what() const throw();
    };

}  // namespace discord
