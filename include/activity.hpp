#pragma once
#include <discord.hpp>

namespace discord {
    class Activity {
        /**
         * @class
         * @brief discord::Activity class, the activity of a user
         * 
         * Construct this class to change the presence of your bot.
         * 
         * ```cpp
         *      bot.change_presence(
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

        struct {
            std::string id; /**< Id of the party */
            size_t current_size; /**< Current size of the party */
            size_t max_size; /**< Maximum size of the party */
        } party; /**< The party of the user, might be default constructed */

        struct {
            std::string large_image; /**< Hash of the large image, will be replaced with an Asset */
            std::string large_text; /**< The large text of the user his activity */
            std::string small_image; /**< Hash of the small image, will be replaced with an Asset */
            std::string small_text; /**< The small text of the user his activity */
        } assets; /**< Assets that belong to this activity */

        struct {
            std::string join; /**< Secret for joining this party */
            std::string spectate; /**< Secret for spectating this game */
            std::string match; /**< Secret for this specific instanced match */
        } secrets; /**< Secrets that belong to this activity */

        bool instance; /**< Whether or not the activity is an instanced game session */
        int flags;     /**< Flags of this activity `OR`d together, describes which fields of this struct are populated, check <a href="https://discordapp.com/developers/docs/topics/gateway#activity-object-activity-flags">this</a> for more info */

        struct {
            discord::datetime start; /**< Time when the activity started */
            discord::datetime end; /**< Time when the activity ends */
        } timestamps; /**< Timestamps for start and/or end of the game */

        std::string state; /**< The user's current party status */
        std::string details; /**< What the player is currently doing */
        snowflake application_id; /**< Application ID for the game */

        std::string url; /**< Stream url, is validated when `type` is presence::activity::streaming */
        std::string name; /**< The activity's name */
        std::string status; /**< Only used for sending a presence to the gateway */

        presence::activity type; /**< The type of the activity */
    };
}  // namespace discord