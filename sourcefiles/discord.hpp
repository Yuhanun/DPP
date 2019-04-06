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

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include "function_types.hpp"
#include "gatewayhandler.hpp"



namespace discord {

    class Bot;
    class Guild;
    class Member;
    class Message;
    class Channel;

    typedef uint64_t discord_id;

    using namespace nlohmann;
    using namespace boost;

    typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
    typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
    typedef Events<
        void(), // READY 
        void(discord::Message, std::exception, std::string), // ERROR
        void(discord::Guild), // GUILD_STATUS
        void(discord::Guild), // GUILD_CREATE
        void(discord::Channel), // CHANNEL_CREATE
        void(discord::Channel), // VOICE_CHANNEL_SELECT
        void(discord::Member, discord::Channel), // VOICE_STATE_CREATE
        void(discord::Member, discord::Channel), // VOICE_STATE_UPDATE
        void(discord::Member, discord::Channel), // VOICE_STATE_DELETE
        void(), // VOICE_SETTINGS_UPDATE
        void(discord::Channel), // VOICE_CONNECTION_STATUS
        void(discord::Member), // SPEAKING_START
        void(discord::Member), // SPEAKING_STOP
        void(discord::Message), // MESSAGE_CREATE
        void(discord::Message), // MESSAGE_UPDATE
        void(discord::Message), // MESSAGE_DELETE
        void(discord::Message), // NOTIFICATION_CREATE
        void(), // CAPTURE_SHORTCUT_CHANGE
        void(), // ACTIVITY_JOIN
        void(), // ACTIVITY_SPECTATE
        void()> function_handler; // ACTIVITY_JOIN_REQUEST

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
        Bot(const std::string&, const std::string);
        template <size_t EVENT, typename FType>
        void register_callback(FType&& func)
        {
            std::get<EVENT>(func_holder.tuple).push_back(std::forward<FType>(func));
        }

        discord::Message send_message(discord_id, std::string);
        void write_to_file(std::string, std::string);
        void on_incoming_packet(websocketpp::connection_hdl, client::message_ptr);
        void handle_gateway();

        void run();
    private:
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

        std::string email;
        std::string avatar;
        std::string username;
        std::string prefix;

        function_handler func_holder;
        std::vector<std::shared_ptr<discord::Guild>> guilds;

    private:
        bool heartbeat_acked;
        json hello_packet;

        std::string session_id;
        std::string token;
        
        long long packet_counter;
        int last_sequence_data;

        client c;
        client::connection_ptr con;


        std::unordered_map<std::string, void(*)> event_map;
        std::thread gateway_thread;
        std::thread heartbeat_thread;
    };

    class Channel : public Object{
        public:
            Channel() = default;
            Channel(discord_id id);

            Channel(std::string);

            discord::Message send(std::string);

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

            // std::vector<discord::PermissionOverwrite> overwrites;

            inline static std::string token;
        private:
            enum channel_type {
                TextChannel,
                VoiceChannel,
                CategoryChannel
            };
    };

    class Emoji : public Object{

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
        // std::vector<discord::Member> members;
        // std::vector<discord::Channel> channels;

        // discord::Member owner;
        discord::Channel afk_channel;
        discord::Channel system_channel;

    };

    class User : public Object{
    public:
        User() = default;
        User(discord_id);

    public:
        bool bot;
        std::string name;
        std::string discriminator;
        std::string avatar;
        std::string avatar_url;
        std::string display_name;
        std::string mention;
    };

    class Member: public User{
    public:
        Member() = default;
        Member(discord_id);
    };

    class Message : public Object{
    public:
        Message() = default;
        Message(discord_id);

        inline static Message from_sent_message(std::string);

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

};
