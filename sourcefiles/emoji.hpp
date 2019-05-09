#pragma once
#include "discord.hpp"
#include "role.hpp"

discord::Emoji::Emoji(nlohmann::json event) {
    id = to_sf(event["id"]);
    name = event["name"];

    if (event.contains("roles")) {
        for (auto const& each : event["roles"]) {
            roles.push_back(discord::Role{ to_sf(each) });
        }
    }

    if (event.contains("user")) {
        user = discord::User{ event["user"] };
    }

    require_colons = get_value(event, "require_colons", false);
    managed = get_value(event, "managed", false);
    animated = get_value(event, "animated", false);

    url = format("%/emojis/%.png", get_image_base_url(), id);
}

discord::Emoji::operator std::string() {
    return format("<%:%:%>", animated ? "a" : "", name, id);
}