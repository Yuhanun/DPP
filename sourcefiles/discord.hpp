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
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include <boost/algorithm/string.hpp>

#include "function_type.hpp"
#include "gatewayhandler.hpp"

#include "cpr/cpr.h"

namespace discord {

    class Bot;
    class User;
    class Role;
    class Color;
    class Guild;
    class Member;
    class Invite;
    class Channel;
    class Message;
    class Activity;
    class EmbedBuilder;
    class PermissionOverwrite;
    class PermissionOverwrites;

    namespace detail {
        inline discord::Bot* bot_instance;
    };  // namespace detail

    typedef uint64_t snowflake;

    using namespace boost;

    typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
    typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

    using websocketpp::lib::bind;

    typedef Events<
        void(),                                   // HELLO
        void(),                                   // READY
        void(),                                   // RESUMED
        void(),                                   // INVALID_SESSION,
        void(discord::Channel),                   // CHANNEL_CREATE
        void(discord::Channel),                   // CHANNEL_UPDATE
        void(discord::Channel),                   // CHANNEL_DELETE
        void(discord::Channel),                   // CHANNEL_PINS_UPDATE
        void(discord::Guild),                     // GUILD_CREATE
        void(discord::Guild),                     // GUILD_UPDATE
        void(discord::Guild),                     // GUILD_DELETE
        void(discord::User),                      // GUILD_BAN_ADD,
        void(discord::User),                      // GUILD_BAN_REMOVE,
        void(discord::Guild),                     // GUILD_EMOJIS_UPDATE
        void(discord::Guild),                     // GUILD_INTEGRATIONS_UPDATE
        void(discord::Member),                    // GUILD_MEMBER_ADD
        void(discord::User),                      // GUILD_MEMBER_REMOVE
        void(discord::Member),                    // GUILD_MEMBER_UPDATE
        void(),                                   // GUILD_MEMBERS_CHUNK
        void(discord::Role),                      // GUILD_ROLE_CREATE
        void(discord::Role),                      // GUILD_ROLE_UPDATE
        void(discord::Role),                      // GUILD_ROLE_DELETE
        void(discord::Message),                   // MESSAGE_CREATE
        void(discord::Message),                   // MESSAGE_UPDATE
        void(discord::Message),                   // MESSAGE_DELETE
        void(std::vector<discord::Message>),      // MESSAGE_DELETE_BULK
        void(discord::Message),                   // MESSAGE_REACTION_ADD
        void(discord::Message),                   // MESSAGE_REACTION_REMOVE
        void(discord::Message),                   // MESSAGE_REACTION_REMOVE_ALL
        void(discord::User),                      // PRECENSE_UPDATE
        void(discord::Member, discord::Channel),  // PRESENCE_UPDATE
        void(discord::User),                      // USER_UPDATE
        void(discord::Member, discord::Channel),  // VOICE_STATE_UPDATE
        void(discord::Guild),                     // VOICE_SERVER_UPDATE
        void(discord::Guild)>                     // WEBHOOKS_UPDATE
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

        friend std::ostream& operator<<(std::ostream& stream, Object& o) {
            stream << o.id;
            return stream;
        }
    };


    class Bot {
    public:
        Bot(std::string const&, const std::string, int = 5000);
        template <size_t EVENT, typename FType>
        void register_callback(FType&& func) {
            std::get<EVENT>(func_holder.tuple).push_back(std::forward<FType>(func));
        }

        void register_command(std::string const&, std::function<void(discord::Message&, std::vector<std::string>&)>);

        void update_presence(Activity const&);

        discord::Message send_message(snowflake, std::string, bool = false);
        discord::Message send_message(snowflake, nlohmann::json, bool = false);
        discord::Guild create_guild(std::string const&, std::string const& = "us-east", int const& = 0, int const& = 0, int const& = 0);

        void on_incoming_packet(websocketpp::connection_hdl, client::message_ptr);
        void handle_gateway();

        void run();

    private:
        void fire_commands(discord::Message&) const;
        void await_events();
        void gateway_auth();
        void handle_heartbeat();
        void handle_event(nlohmann::json const, std::string);
        void initialize_variables(const std::string);
        template <std::size_t event_type>
        discord::Message process_message_cache(discord::Message* m, bool&);

        void guild_create_event(nlohmann::json);
        void channel_create_event(nlohmann::json);
        void channel_update_event(nlohmann::json);
        void channel_delete_event(nlohmann::json);

        std::string get_gateway_url();
        std::string get_identify_packet();
        std::string get_create_guild_url();

        cpr::Header get_basic_header();

    public:
        bool authenticated;
        std::string error_message;
        std::string discriminator;

        long id;


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
        std::vector<std::unique_ptr<discord::User>> users;
        std::vector<std::unique_ptr<discord::Guild>> guilds;
        std::vector<std::unique_ptr<discord::Channel>> channels;

    private:
        int message_cache_count;
        nlohmann::json hello_packet;
        nlohmann::json ready_packet;

        bool heartbeat_acked;
        int last_sequence_data;
        long long packet_counter;

        std::string session_id;

        client c;
        client::connection_ptr con;

        std::thread event_thread;
        std::thread gateway_thread;
        std::thread heartbeat_thread;
        std::future<void> client_future;

        std::vector<discord::Message> messages;
        std::vector<std::future<void>> packet_handling;
        std::unordered_map<std::string, std::function<void(discord::Message&, std::vector<std::string>&)>> command_map;
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

        discord::Message send(std::string, bool = false) const;
        discord::Message send(EmbedBuilder, bool = false, std::string = "") const;
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

    private:
        std::string get_bulk_delete_url();
        std::string get_get_messages_url(int);
        std::string get_channel_edit_url();
        std::string get_delete_url();
        std::string get_get_message_url(snowflake);
        std::string get_channel_invites_url();
        std::string get_create_invite_url() const;
        std::string get_delete_channel_permission_url(discord::Object const&);
        std::string get_typing_url();
        std::string get_pins_url();

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

        discord::Guild* guild;
        std::vector<discord::User> recipients;
        std::vector<discord::PermissionOverwrites> overwrites;
    };

    class User : public Object {
    public:
        User() = default;
        User(snowflake);
        User(nlohmann::json const);

    public:
        bool bot;

        std::string name;

        std::string avatar;
        std::string mention;
        std::string discriminator;
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


    class Member : public User {
    public:
        Member() = default;
        Member(snowflake);
        Member(nlohmann::json const, discord::User const&);

    public:
        bool deaf;
        bool muted;

        std::vector<discord::Role> roles;

        std::string nick;
        std::string joined_at;
    };

    class Invite {
    public:
        Invite() = default;
        Invite(nlohmann::json const);

        int uses;
        int max_age;
        int max_uses;
        bool temporary;

        std::string code;
        std::string created_at;
        discord::Member inviter;
        discord::Channel channel;
    };


    class Guild : public Object {
    public:
        Guild() = default;
        Guild(snowflake);

        Guild(nlohmann::json const);

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
        std::string created_at;
        std::string vanity_url_code;

        std::vector<int> features;
        std::vector<discord::Role> roles;
        std::vector<discord::Emoji> emojis;
        std::vector<discord::Member> members;
        std::vector<discord::Channel> channels;

        discord::Member owner;
        discord::Channel afk_channel;
        discord::Channel system_channel;


    private:
    };

    class Message : public Object {
    public:
        Message() = default;
        Message(snowflake);

        inline static Message from_sent_message(nlohmann::json);
        discord::Message edit(std::string);
        discord::Message edit(EmbedBuilder, std::string = "");
        void pin();
        void unpin();

        std::string get_edit_url();
        std::string get_delete_url();
        std::string get_pin_url();
        std::string get_unpin_url();

        void remove();

    public:
        int type;

        bool tts;
        bool sent;
        bool pinned;
        bool mention_everyone;

        snowflake id;

        std::string content;
        std::string timestamp;
        std::string edited_timestamp;

        discord::Member author;
        discord::Channel channel;

        std::vector<discord::Member> mentions;
        std::vector<discord::Role> mentioned_roles;
        // TODO std::vector<type> attachments;
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
        nlohmann::json& to_json();
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
    };

    class Role : public Object {
    public:
        Role() = default;
        Role(snowflake);
        Role(nlohmann::json);

    public:
        bool hoist;
        bool managed;
        bool mentionable;

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
};  // namespace discord
