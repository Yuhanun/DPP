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

        bool afk;

        struct {
            std::string id;
            size_t current_size;
            size_t max_size;
        } party;

        struct {
            std::string large_image;
            std::string large_text;
            std::string small_image;
            std::string small_text;
        } assets;

        struct {
            std::string join;
            std::string spectate;
            std::string match;
        } secrets;

        bool instance;
        int flags;

        struct {
            discord::datetime start;
            discord::datetime end;
        } timestamps;

        std::string state;
        std::string details;
        snowflake application_id;

        std::string url;
        std::string name;
        std::string status;

        presence::activity type;
    };
}  // namespace discord