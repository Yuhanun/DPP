#pragma once
#include <string>
#include <iostream>
#include <list>
#include <array>
#include <chrono>
#include <thread>
#include <memory>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>

#include <boost/algorithm/string.hpp>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include "function_types.hpp"
#include "gatewayhandler.hpp"



namespace discord {

    class Bot;
    class User;
    class Color;
    class Guild;
    class Member;
    class Message;
    class Channel;
    class EmbedBuilder;
    class PermissionOverwrite;
    class PermissionOverwrites;

    typedef uint64_t discord_id;

    using namespace nlohmann;
    using namespace boost;

    typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
    typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

    typedef Events<
        void(), // HELLO
        void(), // READY 
        void(), // RESUMED
        void(), // INVALID_SESSION,
        void(discord::Channel), // CHANNEL_CREATE
        void(discord::Channel), // CHANNEL_UPDATE
        void(discord::Channel), // CHANNEL_DELETE
        void(discord::Channel), // CHANNEL_PINS_UPDATE
        void(discord::Guild), // GUILD_CREATE
        void(discord::Guild), // GUILD_UPDATE
        void(discord_id), // GUILD_DELETE
        void(discord::User), // GUILD_BAN_ADD,
        void(discord::User), // GUILD_BAN_REMOVE,
        void(discord::Guild), // GUILD_EMOJIS_UPDATE
        void(discord::Guild), // GUILD_INTEGRATIONS_UPDATE
        void(discord::Member), // GUILD_MEMBER_ADD
        void(), // GUILD_MEMBERS_CHUNK
        void(), //discord::Role), // GUILD_ROLE_CREATE 
        void(), //discord::Role), // GUILD_ROLE_UPDATE 
        void(), //discord::Role), // GUILD_ROLE_DELETE
        void(discord::Message), // MESSAGE_CREATE
        void(discord::Message), // MESSAGE_UPDATE
        void(discord::Message), // MESSAGE_DELETE
        void(std::vector<discord::Message>), //MESSAGE_DELETE_BULK
        void(discord::Message), // MESSAGE_REACTION_ADD
        void(discord::Message), // MESSAGE_REACTION_REMOVE
        void(discord::Message), // MESSAGE_REACTION_REMOVE_ALL
        void(discord::User), // PRECENSE_UPDATE
        void(discord::Member, discord::Channel), // PRESENCE_UPDATE
        void(discord::User), // USER_UPDATE
        void(discord::Member, discord::Channel), // VOICE_STATE_UPDATE
        void(discord::Guild), // VOICE_SERVER_UPDATE
        void(discord::Guild)> function_handler; // WEBHOOKS_UPDATE

    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;
    using websocketpp::lib::bind;

    class Object {
    public:

        Object() = default;

        Object(discord_id id)
            : id{id}
        {}

        discord_id id;
        // some datetime creation time.


        bool operator==(const Object& other){
            return this->id == other.id;
        }

        bool operator==(const discord_id& other){
            return this->id == other;
        }
        template <typename T>
        bool operator!=(T&& other){
            return !(std::forward<T>(other) == this);
        }

        friend std::ostream& operator<<(std::ostream& stream, Object& o){
            stream << o.id;
            return stream;
        }
    };


    class Bot {
    public:
        Bot(std::string const&, const std::string);
        template <size_t EVENT, typename FType>
        void register_callback(FType&& func)
        {
            std::get<EVENT>(func_holder.tuple).push_back(std::forward<FType>(func));
        }

        void register_command(std::string const&, std::function<void(discord::Message&, std::vector<std::string>&)>);

        discord::Message send_message(discord_id, std::string);
        discord::Message send_message(discord_id, json);

        void write_to_file(std::string, std::string);
        void on_incoming_packet(websocketpp::connection_hdl, client::message_ptr);
        void handle_gateway();

        void run();

    private:
        void fire_commands(discord::Message&) const;
        void gateway_auth();
        void handle_heartbeat();
        void handle_event(json&, std::string);
        void initialize_variables(const std::string);
        
        std::string get_gateway_url();
        std::string get_identify_packet();
        
        std::list<std::string> get_basic_header();

    public:
        bool authenticated;
        std::string error_message;

        long id;

        int discriminator;

        bool bot;
        bool ready = false;
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
        bool heartbeat_acked;
        json hello_packet;

        std::string session_id;
        
        long long packet_counter;
        int last_sequence_data;

        client c;
        client::connection_ptr con;

        std::thread gateway_thread;
        std::thread heartbeat_thread;

        std::unordered_map<std::string, std::function<void(discord::Message&, std::vector<std::string>&)>> command_map;
    };

    class Channel : public Object{
        public:
            Channel() = default;
            Channel(discord_id id);

            Channel(std::string, discord_id);

            discord::Message send(std::string);
            discord::Message send(EmbedBuilder, std::string="");

        public:
            int type;
            int bitrate;
            int position;
            int parent_id;
            int user_limit;
            int rate_limit_per_user;

            discord_id last_message_id;

            std::string name;
            std::string topic;

            discord::Guild* guild;
            std::vector<discord::PermissionOverwrites> overwrites;

            inline static discord::Bot* bot;

        private:
            enum channel_type {
                TextChannel,
                VoiceChannel,
                CategoryChannel
            };
    };

    class Emoji : public Object{

    };



    class User : public Object{
    public:
        User() = default;
        User(discord_id);
        User(std::string const&);

    public:
        bool bot;
        
        std::string name;

        std::string avatar;
        std::string mention;
        std::string discriminator;
    };

    class Member: public User{
    public:
        Member() = default;
        Member(discord_id);
        Member(std::string const&, discord::User const&);

    public:
        bool deaf;
        bool muted;
        
        // std::vector<discord::Role> roles;

        std::string nick;
        std::string joined_at;
    };


    class Guild : public Object{
    public:
        Guild() = default;
        Guild(discord_id);

        Guild(std::string);

    public:
        int splash;
        int mfa_level;
        int afk_timeout;
        int member_count;
        int verification_level;
        int explicit_content_filter;

        bool large;
        bool unavailable;

        discord_id id;
        discord_id application_id;

        std::string name;
        std::string icon;
        std::string region;
        std::string banner;
        std::string created_at;
        std::string vanity_url_code;

        std::vector<int> features;
        // std::vector<discord::Role> roles;
        // std::vector<discord::Emoji> emojis;
        std::vector<discord::Member> members;
        std::vector<discord::Channel> channels;

        discord::Member owner;
        discord::Channel afk_channel;
        discord::Channel system_channel;

        inline static discord::Bot* bot;
    
    private:

    };
    
    class Message : public Object{
    public:
        Message() = default;
        Message(discord_id);

        inline static Message from_sent_message(std::string, discord::Bot*);

        std::string get_delete_url();

        json remove();

    public:
        int type;
        int error_code;

        bool tts;
        bool sent;
        bool pinned;
        bool mention_everyone;

        discord_id id;

        std::string error;
        std::string content;
        std::string timestamp;
        // std::string edited_timestamp;
        
        discord::Member author;
        discord::Channel channel;
        
        // std::vector<std::string> mentions;
        // std::vector<discord::Role> mentioned_roles;
        // std::vector<type> attachments; 
        // std::vector<embed> embeds;
    
    private:
        std::string token;

    };

    class Role : public Object{
        
    };

    class EmbedBuilder{
    public:
        EmbedBuilder();
        EmbedBuilder& set_title(std::string const&);
        EmbedBuilder& set_description(std::string const&);
        EmbedBuilder& set_url(std::string const&);
        EmbedBuilder& set_timestamp(std::string const&);
        EmbedBuilder& set_color(const Color);
        EmbedBuilder& set_footer(std::string const&, std::string const&);
        EmbedBuilder& set_image(std::string const&, const int=-1, const int=-1);
        EmbedBuilder& set_thumbnail(std::string const&, const int=-1, const int=-1);
        EmbedBuilder& set_video(std::string const&, const int=-1, const int=-1);
        EmbedBuilder& set_author(std::string const&, std::string const&, std::string const&);
        EmbedBuilder& add_field(std::string const&, std::string const&, const bool=false);
        json& to_json();

    private:
        json embed;
    };

    class Color{
    public:
        Color() = default;
        Color(int, int, int, int=-1);
        Color(int);

    public:
        int raw_int;
        int r;
        int g;
        int b;
    };

    class PermissionOverwrite{
    public:
        PermissionOverwrite(std::string const&, bool);
    public:
        std::string name;
        bool value;
        int hex_value;

    };

    class PermissionOverwrites{
    public:
        PermissionOverwrites() = default;
        PermissionOverwrites(const int);
        PermissionOverwrites(std::vector<PermissionOverwrite> const&);
        PermissionOverwrites(const int&, discord_id, std::string const&);

        PermissionOverwrites& remove_overwrite(std::string const&);
        PermissionOverwrites& add_overwrite(PermissionOverwrite const&);

        discord::PermissionOverwrite get_overwrite(std::string const&) const;

        std::vector<discord::PermissionOverwrite>::const_iterator begin() const noexcept;
        std::vector<discord::PermissionOverwrite>::const_iterator end() const noexcept;

        int get_value() const;

    private:
        void calculate_value();

    public:
        std::vector<PermissionOverwrite> overwrites;
        discord_id object_id;
        int type;

    private:
        int value;
        enum owner_type{
            role,
            member
        };
    };



    class ImproperToken : public std::exception{
        const char* what() const throw();
    };

};
