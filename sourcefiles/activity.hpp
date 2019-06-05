#pragma once
#include "discord.hpp"
#include "utility.hpp"


discord::Activity::Activity(std::string const& name, presence::activity const& type, std::string const& status, bool const& afk, std::string const& url)
    : afk{ afk }, url{ url }, name{ name }, status{ status }, type{ type } {
}

discord::Activity::Activity(nlohmann::json const data) {
    name = data["name"];
    type = data["type"];
    if (static_cast<int>(type) == 1) url = data["url"];

    auto ts_obj = get_value(data, "timestamps", nlohmann::json({}));
    timestamps.start = time_from_discord_string(get_value(ts_obj, "start", ""));
    timestamps.end = time_from_discord_string(get_value(ts_obj, "end", ""));

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
    auto payload = nlohmann::json({ { "game", { { "name", name }, { "type", static_cast<int>(type) } } },
                                    { "status", status },
                                    { "afk", afk },
                                    { "since", nullptr } });
    if (type == presence::activity::streaming) {
        payload["game"]["url"] = url;
    }
    return payload;
}