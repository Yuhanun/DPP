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
#include <variant>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include <boost/algorithm/string.hpp>

#include "events.hpp"
#include "function_type.hpp"
#include "gatewayhandler.hpp"

#include "cpr/cpr.h"

namespace discord {

    class Bot;
    class User;
    class Role;
    class Color;
    class Emoji;
    class Guild;
    class Asset;
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

    enum object_type {
        role,
        member
    };

    using namespace boost;

    typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
    typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

    using websocketpp::lib::bind;

    typedef Events<
        void(),                                                                             // HELLO
        void(),                                                                             // READY
        void(),                                                                             // RESUMED
        void(),                                                                             // INVALID_SESSION,
        void(std::shared_ptr<discord::Channel> const),                                      // CHANNEL_CREATE
        void(std::shared_ptr<discord::Channel> const),                                      // CHANNEL_UPDATE
        void(std::shared_ptr<discord::Channel> const),                                      // CHANNEL_DELETE
        void(std::shared_ptr<discord::Channel> const, datetime const),                      // CHANNEL_PINS_UPDATE
        void(std::shared_ptr<discord::Guild> const),                                        // GUILD_CREATE
        void(std::shared_ptr<discord::Guild> const),                                        // GUILD_UPDATE
        void(std::shared_ptr<discord::Guild> const),                                        // GUILD_DELETE
        void(std::shared_ptr<discord::Guild> const, std::shared_ptr<discord::User> const),  // GUILD_BAN_ADD,
        void(std::shared_ptr<discord::Guild> const, std::shared_ptr<discord::User> const),  // GUILD_BAN_REMOVE,
        void(std::shared_ptr<discord::Guild> const),                                        // GUILD_EMOJIS_UPDATE
        void(std::shared_ptr<discord::Guild> const),                                        // GUILD_INTEGRATIONS_UPDATE
        void(std::shared_ptr<discord::Member> const),                                       // GUILD_MEMBER_ADD
        void(std::shared_ptr<discord::Guild> const, std::shared_ptr<discord::User> const),  // GUILD_MEMBER_REMOVE
        void(std::shared_ptr<discord::Member> const),                                       // GUILD_MEMBER_UPDATE
        void(),                                                                             // GUILD_MEMBERS_CHUNK
        void(discord::Role const),                                                          // GUILD_ROLE_CREATE
        void(discord::Role const),                                                          // GUILD_ROLE_UPDATE
        void(discord::Role const),                                                          // GUILD_ROLE_DELETE
        void(std::shared_ptr<discord::Message> const),                                      // MESSAGE_CREATE
        void(std::shared_ptr<discord::Message> const),                                      // MESSAGE_UPDATE
        void(std::shared_ptr<discord::Message> const),                                      // MESSAGE_DELETE
        void(std::vector<std::shared_ptr<discord::Message>> const),                         // MESSAGE_DELETE_BULK
        void(std::shared_ptr<discord::Message> const, discord::Emoji const),                // MESSAGE_REACTION_ADD
        void(std::shared_ptr<discord::Message> const, discord::Emoji const),                // MESSAGE_REACTION_REMOVE
        void(std::shared_ptr<discord::Message> const),                                      // MESSAGE_REACTION_REMOVE_ALL
        void(discord::User),                                                                // PRECENSE_UPDATE
        void(discord::Member, discord::Channel),                                            // PRESENCE_UPDATE
        void(discord::User),                                                                // USER_UPDATE
        void(discord::Member, discord::Channel),                                            // VOICE_STATE_UPDATE
        void(discord::Guild),                                                               // VOICE_SERVER_UPDATE
        void(discord::Guild),                                                               // WEBHOOKS_UPDATE
        void(snowflake, snowflake, nlohmann::json const),                                   // RAW_MESSAGE_UPDATE
        void(snowflake, snowflake, nlohmann::json const),                                   // RAW_MESSAGE_DELETE
        void(snowflake, nlohmann::json const)>                                              // RAW_MESSAGE_DELETE_BULK
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

        bool operator==(const Object& other) const {
            return this->id == other.id;
        }

        bool operator==(const snowflake& other) const {
            return this->id == other;
        }
        template <typename T>
        bool operator!=(T&& other) const {
            return !(std::forward<T>(other) == this);
        }

        explicit operator snowflake() const {
            return id;
        }

        friend std::ostream& operator<<(std::ostream& stream, Object const& o) {
            return stream << "<discord::Object id=" << o.id << " at " << &o << ">";
        }
    };

    class Asset {
    public:
        std::string byte_arr;
        bool gotten_data;
        snowflake obj_id;
        std::string url;
        int asset_type;
        bool _animated;

        Asset() = default;
        Asset(std::string const&, int, bool = false, snowflake = 0);

        int type() const;
        std::string read();
        size_t len() const;
        bool animated() const;
        std::string hash() const;
        snowflake object_id() const;
        void save(std::string const&);
        explicit operator bool() const;
        explicit operator std::string() const;
        bool operator==(Asset const&) const;
        bool operator!=(Asset const&) const;
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

    struct File {
        std::string filename;
        std::string filepath;
        bool spoiler;
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

        std::string mention;
        discord::Asset avatar;
        std::string discriminator;
    };

    class Member : public User {
    public:
        Member() = default;
        Member(snowflake);
        Member(nlohmann::json const, discord::User const&, discord::Guild*);
        Member& update(nlohmann::json const);  // TODO

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

        int run();

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
        std::future<void> client_future;

        std::vector<std::future<void>> futures;
        std::vector<std::shared_ptr<discord::Message>> messages;
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

        Channel& update(nlohmann::json const);

        discord::Message send(std::string const&, std::vector<File> const& = {}, bool = false) const;
        discord::Message send(EmbedBuilder const&, std::vector<File> const& = {}, bool = false, std::string const& = "") const;
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
        bool is_custom;
        bool require_colons;

        snowflake id;
        std::string name;
        discord::User user;
        discord::Asset image;
        std::vector<discord::Role> roles;
    };

    struct AuditLogKeyChange {
        AuditLogKeyChange(const nlohmann::json&, snowflake);
        AuditLogKeyChange() = default;
        std::string name;
        discord::Asset icon;
        discord::Asset splash;
        discord::User owner;  //
        std::string region;
        std::shared_ptr<discord::Channel> afk_channel;  //
        int afk_timeout;
        int mfa_level;
        int verification_level;
        int explicit_content_filter;
        int default_message_notifications;
        std::string vanity_url_code;
        int prune_delete_days;
        bool widget_enabled;
        std::shared_ptr<discord::Channel> widget_channel;  //
        int position;
        std::string topic;
        int bitrate;
        bool nsfw;
        snowflake application_id;
        std::string permissions;
        std::vector<discord::PermissionOverwrites> permission_overwrites;  //
        int color;
        bool hoist;
        bool mentionable;
        int allow;
        int deny;
        std::string code;
        std::shared_ptr<discord::Channel> channel;
        int max_uses;
        int uses;
        int max_age;
        bool temporary;
        bool deaf;
        bool mute;
        std::string nick;
        discord::Asset avatar;
        snowflake id;
        std::string type;
    };

    class AuditLogEntry {
    public:
        AuditLogEntry() = default;
        AuditLogEntry(nlohmann::json const&);

        snowflake target_id;
        std::vector<AuditLogKeyChange> changes;
        snowflake user_id;
        snowflake id;
        AuditLogEventType action_type;
        int count;
        object_type type;
        std::string reason;

        struct {
            std::string delete_member_days;
            std::string members_removed;
            snowflake channel_id;
            std::string count;
            snowflake id;
            std::string type;
            std::string role_name;
        } opts;
        std::vector<decltype(opts)> options;
    };

    struct AuditLogs {
        AuditLogs() = default;
        AuditLogs(const nlohmann::json&);
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
    };


    class Guild : public Object {
    public:
        Guild() = default;
        Guild(snowflake);

        Guild(nlohmann::json const);
        Guild& update(nlohmann::json const);  // TODO

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

        discord::Emoji create_emoji(std::string const&, discord::Emoji&, std::vector<discord::Role> = {});

        std::vector<discord::Role> get_roles();

        discord::Role create_role(std::string const&, PermissionOverwrites&, discord::Color, bool, bool);

        int get_prune_count(int);
        int begin_prune(int, bool);

        std::vector<discord::VoiceRegion> get_voice_regions();
        std::vector<discord::Invite> get_invites();

        snowflake get_embed();
        snowflake edit_embed(snowflake = -1);

        std::string get_vanity_invite_url();

        discord::Asset get_widget_image(std::string const&);
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
        std::string region;
        discord::Asset icon;
        discord::Asset banner;
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
        discord::Emoji edit_emoji(discord::Emoji const&, std::string, std::vector<discord::Role> = {});
        void remove_emoji(discord::Emoji const&);
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
        discord::Asset avatar;
        std::string token;

        // TODO: avatar edit for both
        void edit(std::string const& = "", snowflake = 0);
        void edit(std::string const& = "");

        void remove();

        discord::Message send(std::string const&, bool = false, std::string const& = "", std::string const& = "");
        discord::Message send(std::vector<EmbedBuilder> const&, bool = false, std::string const& = "", std::string const& = "", std::string const& = "");

        void execute_slack(bool, nlohmann::json const);
        void execute_github(bool, nlohmann::json const);
    };

    class Message : public Object {
    public:
        Message() = default;
        Message(snowflake);
        Message(nlohmann::json const);
        Message& update(nlohmann::json const);  // TODO

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
        EmbedBuilder& set_color(const discord::Color);
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

        bool has_permission(std::string const&);
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
        // PermissionOverwrites(int, snowflake, int);

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
        Role(nlohmann::json, std::shared_ptr<discord::Guild>);
        Role& update(nlohmann::json const);

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

    struct Err {
        std::string error;
        int resp_code;
        Err(std::string err, int err_code)
            : error{ std::move(err) }, resp_code{ err_code } {
        }
    };

    template <typename T>
    struct Ok {
        Ok(T val)
            : value{ std::move(val) } {
        }
        T value;
    };

    template <typename T>
    struct Result {
    private:
        std::variant<Err, T> var_obj;
        bool _is_ok;

    public:
        Result(Ok<T> const& value)
            : _is_ok{ true }, var_obj{ value.value } {
        }

        Result(Err const& error)
            : _is_ok{ false }, var_obj{ error } {
        }

        ~Result(){};

        Result& operator=(Result&& other) = default;
        Result& operator=(Result const& other) = default;

        Result(Result&& other) {
            _is_ok = other.is_ok();
            if (is_ok()) {
                var_obj = std::move(other.val);
            } else {
                var_obj = std::move(other.err);
            }
        }

        Result(Result const& other) {
            _is_ok = other.is_ok();
            if (is_ok()) {
                var_obj = other.val;
            } else {
                var_obj = other.err;
            }
        }

        auto is_ok() const noexcept -> bool {
            return _is_ok;
        }

        auto is_err() const noexcept -> bool {
            return !is_ok();
        }

        auto unwrap() const -> T const& {
            if (is_ok()) {
                return std::get<T>(var_obj);
            }
            throw std::runtime_error{ std::to_string(std::get<Err>(var_obj).resp_code) + std::string(": ") + std::get<Err>(var_obj).error };
        }

        auto unwrap_or(T const& value) const noexcept -> T const& {
            if (is_ok()) {
                return std::get<T>(var_obj);
            }
            return value;
        }

        auto expect(std::string const& msg) const -> T const& {
            if (!is_ok()) {
                throw std::runtime_error{ msg };
            }
            return std::get<T>(var_obj);
        }

        auto unwrap_err() const -> Err {
            if (is_ok()) {
                throw std::runtime_error{
                    "Error was unwrapped when contained value was \"Ok\""
                };
            }
            return std::get<Err>(var_obj);
        }

        auto expect_err(std::string const& msg) const -> Err const& {
            if (!is_err()) {
                throw std::runtime_error{ msg };
            }
            return std::get<Err>(var_obj);
        }
    };

    class ImproperToken : public std::exception {
        const char* what() const throw();
    };
}  // namespace discord