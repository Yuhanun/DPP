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
    std::optional<nlohmann::json> ts_obj = get_value_optional<nlohmann::json>(data, "timestamps");

    if (ts_obj.has_value()) {
        auto millis = get_value_optional<size_t>(ts_obj.value(), "start");
        if (millis.has_value()) {
            if (millis != 0) {
                timestamps->start = boost::posix_time::from_time_t(millis.value() / 1000) + boost::posix_time::millisec(millis.value() % 1000);
            }
        }
        millis = get_value_optional<size_t>(ts_obj.value(), "end");
        if (millis.has_value()) {
            if (millis != 0) {
                timestamps->end = boost::posix_time::from_time_t(millis.value() / 1000) + boost::posix_time::millisec(millis.value() % 1000);
            }
        }
    }

    application_id = get_value_optional<snowflake>(data, "application_id");
    details = get_value_optional<std::string>(data, "details");
    state = get_value_optional<std::string>(data, "state");

    auto party_obj = get_value_optional<nlohmann::json>(data, "party");
    if (party_obj.has_value()) {
        party->id = get_value_optional<std::string>(party_obj.value(), "id");
        auto size = get_value_optional<nlohmann::json>(party_obj.value(), "size");
        if (size.has_value()) {
            party->current_size = size.value()[0];
            party->max_size = size.value()[1];
        }
    }

    auto assets_obj = get_value_optional<nlohmann::json>(data, "assets");
    if (assets_obj.has_value()) {
        assets->large_image = get_value_optional<std::string>(assets_obj.value(), "large_image");
        assets->large_text = get_value_optional<std::string>(assets_obj.value(), "large_text");
        assets->small_image = get_value_optional<std::string>(assets_obj.value(), "small_image");
        assets->small_text = get_value_optional<std::string>(assets_obj.value(), "small_text");
    }

    auto secrets_obj = get_value_optional<nlohmann::json>(data, "secrets");
    if (secrets_obj.has_value()) {
        secrets->join = get_value_optional<std::string>(secrets_obj.value(), "join");
        secrets->spectate = get_value_optional<std::string>(secrets_obj.value(), "spectate");
        secrets->spectate = get_value_optional<std::string>(secrets_obj.value(), "match");
    }

    instance = get_value_optional<bool>(data, "instance");
    flags = get_value_optional<int>(data, "flags");
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
        payload["game"]["url"] = url.value();
    }
    return payload;
}