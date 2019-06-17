#pragma once
#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace discord {

    typedef int64_t snowflake;

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

    class Activity {
        /**
         * @class
         * @brief discord::Activity class, the activity of a user
         * 
         * Construct this class to change the presence of your bot.
         * 
         * ```cpp
         *      bot.update_presence(
         *          discord::Activity{
         *              discord::format("to % guilds", bot.guilds.size()),
         *              presence::activity::listening,
         *              presence::status::dnd,
         *              false
         *          }
         *      )
         * ```
         * 
         * @throws Anything any of its members can throw.
         */

    public:
        Activity() = default;
        Activity(nlohmann::json const);
        Activity(std::string const&, presence::activity const&, std::string const& = "online", bool const& = false, std::string const& = "");

        nlohmann::json to_json() const;

        bool afk; /**< Whether the user this activity belongs to is afk */

        struct party_t {
            std::optional<std::string> id;      /**< Id of the party */
            std::optional<size_t> current_size; /**< Current size of the party */
            std::optional<size_t> max_size;     /**< Maximum size of the party */
        };

        std::optional<party_t> party; /**< The party of the user, might be default constructed */

        struct asset_t {
            std::optional<std::string> large_image; /**< Hash of the large image, will be replaced with an Asset */
            std::optional<std::string> large_text;  /**< The large text of the user his activity */
            std::optional<std::string> small_image; /**< Hash of the small image, will be replaced with an Asset */
            std::optional<std::string> small_text;  /**< The small text of the user his activity */
        };

        std::optional<asset_t> assets; /**< Assets that belong to this activity */

        struct secret {
            std::optional<std::string> join;     /**< Secret for joining this party */
            std::optional<std::string> spectate; /**< Secret for spectating this game */
            std::optional<std::string> match;    /**< Secret for this specific instanced match */
        };

        std::optional<secret> secrets; /**< Secrets that belong to this activity */

        std::optional<bool> instance; /**< Whether or not the activity is an instanced game session */
        std::optional<int> flags;     /**< Flags of this activity `OR`d together, describes which fields of this struct are populated, check <a href="https://discordapp.com/developers/docs/topics/gateway#activity-object-activity-flags">this</a> for more info */

        struct timestamp {
            std::optional<boost::posix_time::ptime> start; /**< Time when the activity started */
            std::optional<boost::posix_time::ptime> end;   /**< Time when the activity ends */
        };

        std::optional<timestamp> timestamps; /**< Timestamps for start and/or end of the game */

        std::optional<std::string> state;        /**< The user's current party status */
        std::optional<std::string> details;      /**< What the player is currently doing */
        std::optional<snowflake> application_id; /**< Application ID for the game */

        std::optional<std::string> url; /**< Stream url, is validated when `type` is presence::activity::streaming */
        std::string name;               /**< The activity's name */
        std::string status;             /**< Only used for sending a presence to the gateway */

        presence::activity type; /**< The type of the activity */
    };
}  // namespace discord