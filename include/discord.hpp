#pragma once
#include "function_type.hpp"

#include <boost/date_time/local_time/local_time.hpp>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include <cpprest/http_client.h>


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
    class Presence;
    class Attachment;
    class Integration;
    class EmbedBuilder;
    class AuditLogEntry;
    class PermissionOverwrite;
    class PermissionOverwrites;

    template <typename T>
    struct Result;
    struct Context;
    struct AuditLogs;
    struct AuditLogKeyChange;

    namespace detail {
        inline discord::Bot* bot_instance; /**< Global instance of your bot */
    }                                      // namespace detail

    typedef int64_t snowflake;
    typedef boost::posix_time::ptime datetime;

    enum object_type {
        /**
         * @brief Enum used for object types, either role or member
         */
        role,
        member
    };

    using namespace boost;

    typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
    typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

    using namespace web;
    using namespace web::http;
    using namespace web::http::client;

    using websocketpp::lib::bind;

    typedef Events<
        void(),                                                                                         // HELLO
        void(),                                                                                         // READY
        void(),                                                                                         // RESUMED
        void(),                                                                                         // INVALID_SESSION,
        void(std::shared_ptr<discord::Channel> const),                                                  // CHANNEL_CREATE
        void(std::shared_ptr<discord::Channel> const),                                                  // CHANNEL_UPDATE
        void(std::shared_ptr<discord::Channel> const),                                                  // CHANNEL_DELETE
        void(std::shared_ptr<discord::Channel> const, datetime const),                                  // CHANNEL_PINS_UPDATE
        void(std::shared_ptr<discord::Guild> const),                                                    // GUILD_CREATE
        void(std::shared_ptr<discord::Guild> const),                                                    // GUILD_UPDATE
        void(std::shared_ptr<discord::Guild> const),                                                    // GUILD_DELETE
        void(std::shared_ptr<discord::Guild> const, std::shared_ptr<discord::User> const),              // GUILD_BAN_ADD,
        void(std::shared_ptr<discord::Guild> const, std::shared_ptr<discord::User> const),              // GUILD_BAN_REMOVE,
        void(std::shared_ptr<discord::Guild> const),                                                    // GUILD_EMOJIS_UPDATE
        void(std::shared_ptr<discord::Guild> const),                                                    // GUILD_INTEGRATIONS_UPDATE
        void(std::shared_ptr<discord::Member> const),                                                   // GUILD_MEMBER_ADD
        void(std::shared_ptr<discord::Guild> const, std::shared_ptr<discord::User> const),              // GUILD_MEMBER_REMOVE
        void(std::shared_ptr<discord::Member> const),                                                   // GUILD_MEMBER_UPDATE
        void(),                                                                                         // GUILD_MEMBERS_CHUNK
        void(std::shared_ptr<discord::Role> const),                                                     // GUILD_ROLE_CREATE
        void(std::shared_ptr<discord::Role> const),                                                     // GUILD_ROLE_UPDATE
        void(std::shared_ptr<discord::Role> const),                                                     // GUILD_ROLE_DELETE
        void(std::shared_ptr<discord::Message> const),                                                  // MESSAGE_CREATE
        void(std::shared_ptr<discord::Message> const),                                                  // MESSAGE_UPDATE
        void(std::shared_ptr<discord::Message> const),                                                  // MESSAGE_DELETE
        void(std::vector<std::shared_ptr<discord::Message>> const),                                     // MESSAGE_DELETE_BULK
        void(std::shared_ptr<discord::Message> const, discord::Emoji const),                            // MESSAGE_REACTION_ADD
        void(std::shared_ptr<discord::Message> const, discord::Emoji const),                            // MESSAGE_REACTION_REMOVE
        void(std::shared_ptr<discord::Message> const),                                                  // MESSAGE_REACTION_REMOVE_ALL
        void(std::shared_ptr<discord::Member> const),                                                   // PRECENSE_UPDATE
        void(std::shared_ptr<discord::User> const, std::shared_ptr<discord::Channel> const, datetime),  // TYPING_START
        void(std::shared_ptr<discord::User> const),                                                     // USER_UPDATE
        void(nlohmann::json const),                                                                     // VOICE_STATE_UPDATE
        void(nlohmann::json const),                                                                     // VOICE_SERVER_UPDATE
        void(std::shared_ptr<discord::Channel> const),                                                  // WEBHOOKS_UPDATE
        void(snowflake, snowflake, nlohmann::json const),                                               // RAW_MESSAGE_UPDATE
        void(snowflake, snowflake, nlohmann::json const),                                               // RAW_MESSAGE_DELETE
        void(snowflake, nlohmann::json const)>                                                          // RAW_MESSAGE_DELETE_BULK
        function_handler;

    using websocketpp::lib::bind;
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;

    namespace presence {
        struct status {
            /**
             * @brief Struct used for presence updates
             */
            inline const static std::string online = "online";
            inline const static std::string dnd = "dnd";
            inline const static std::string idle = "idle";
            inline const static std::string invisible = "invisible";
            inline const static std::string offline = "offline";
        };

        enum class activity : short {
            /**
             * @brief Enum used for presence updates
             */
            playing,
            streaming,
            listening,
            watching
        };
    }  // namespace presence

    struct VoiceRegion {
        /**
         * @brief Struct that represents a voice region
         */
        std::string id;   /**< Id of the voice region */
        std::string name; /**< Name of the voice region */
        bool vip;         /**< Whether or not this voice region is VIP server only */
        bool optimal;     /**< Whether this is the optimal voice region for your bot instance */
        bool deprecated;  /**< Whether this voice region is deprecated */
        bool custom;      /**< Whether this is a custom voice region */
    };

    struct Connection {
        /**
         * @brief Represents a connection with a user, for example twitch.
         */
        snowflake id;                          /**< Snowflake, id, of this connection */
        std::string name;                      /**< Name of this connection */
        std::string type;                      /**< Type of this connection */
        bool revoked;                          /**< Whether this connection has been revoked */
        std::vector<Integration> integrations; /**< A vector of integration objects */
        bool verified;                         /**< Whether this connection has been verified */
        bool friend_sync;                      /**< Whether friend sync is enabled for this connection */
        bool show_activity;                    /**< Whether this connection shows activity */
        int visibility;                        /**< Visibility level for this connection */
    };

    struct File {
        /**
         * @brief Struct used for sending files
         * 
         * ```cpp
         *      channel.send("hello!", { { "file.txt", "/usr/bin/test.txt", true } });
         */
        std::string filename; /**< Filename that discord should display */
        std::string filepath; /**< Path to your local file */
        bool spoiler = false; /**< Whether or not this file is spoilered */
    };

    struct RateLimit {
        /**
         * @brief Internal ratelimit struct
         */
        int rate_limit_limit = 500;            /**< Maximum amount of requests for this bucket */
        int rate_limit_remaining = 500;        /**< Remaining request count for this bucker */
        datetime ratelimit_reset = datetime{}; /**< Datetime at which the bucket resets */
    };

    typedef pplx::task<Result<nlohmann::json>> request_response;

}  // namespace discord