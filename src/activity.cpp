#include "activity.hpp"
#include <nlohmann/json.hpp>
#include "bot.hpp"
#include "utils.hpp"

discord::Activity::Activity(std::string const& name, presence::activity const& type, std::string const& status, bool const& afk, std::string const& url)
    : afk{ afk }, url{ url }, name{ name }, status{ status }, type{ type } {
    /**
    * @brief Constructor that should be used for updating your bot's presence
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
    */
}

discord::Activity::Activity(nlohmann::json const data) {
    /**
     * @brief Internal function used for constructing discord::Activity objects.
     */
    name = data["name"];
    type = data["type"];

    if (static_cast<int>(type) == 1) {
        if (data.contains("url")) {
            url = data["url"];
        }
    }
    auto ts_obj = get_value(data, "timestamps", nlohmann::json({}));

    auto millis = get_value(ts_obj, "start", 0);
    if (millis != 0) {
        timestamps.start = boost::posix_time::from_time_t(millis / 1000) + boost::posix_time::millisec(millis % 1000);
    }
    millis = get_value(ts_obj, "end", 0);
    if (millis != 0) {
        timestamps.end = boost::posix_time::from_time_t(millis / 1000) + boost::posix_time::millisec(millis % 1000);
    }

    application_id = to_sf(get_value(data, "application_id", "0"));
    details = get_value(data, "details", "");
    state = get_value(data, "state", "");

    auto party_obj = get_value(data, "party", nlohmann::json({}));
    party.id = get_value(party_obj, "id", "");
    party.current_size = get_value(party_obj, "size", nlohmann::json({ 0, 0 }))[0];
    party.max_size = get_value(party_obj, "size", nlohmann::json({ 0, 0 }))[1];

    auto assets_obj = get_value(data, "assets", nlohmann::json({}));
    assets.large_image = get_value(assets_obj, "large_image", "");
    assets.large_text = get_value(assets_obj, "large_text", "");
    assets.small_image = get_value(assets_obj, "small_image", "");
    assets.small_text = get_value(assets_obj, "small_text", "");

    auto secrets_obj = get_value(data, "secrets", nlohmann::json({}));
    secrets.join = get_value(secrets_obj, "join", "");
    secrets.spectate = get_value(secrets_obj, "spectate", "");
    secrets.spectate = get_value(secrets_obj, "match", "");

    instance = get_value(data, "instance", false);
    flags = get_value(data, "flags", 0);
}


nlohmann::json discord::Activity::to_json() const {
    /**
     * @brief Creates JSON that's able to be sent to the server for updating presence
     * 
     * Shouldn't have to be used by a user.
     * 
     * @return Returns a json payload, ready to send to the server
     */
    auto payload = nlohmann::json({ { "game", { { "name", name }, { "type", static_cast<int>(type) } } },
                                    { "status", status },
                                    { "afk", afk },
                                    { "since", nullptr } });
    if (type == presence::activity::streaming) {
        payload["game"]["url"] = url;
    }
    return payload;
}