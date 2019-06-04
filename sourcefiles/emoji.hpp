#pragma once
#include "discord.hpp"
#include "role.hpp"

discord::Emoji::Emoji(nlohmann::json event) {
    id = to_sf(event["id"]);
    name = event["name"];

    if (event.contains("roles")) {
        for (auto const& each : event["roles"]) {
            roles.emplace_back(to_sf(each));
        }
    }

    if (event.contains("user")) {
        user = discord::User{ event["user"] };
    }

    is_custom = event.contains("require_colons") && event.contains("animated");
    require_colons = get_value(event, "require_colons", false);
    managed = get_value(event, "managed", false);
    animated = get_value(event, "animated", false);

    image = discord::Asset{};
    image.url = image_url_from_type(custom_emoji, id, "", animated);
    image.obj_id = id;
    image.asset_type = custom_emoji;
    image._animated = animated;
}

discord::Emoji::operator std::string() {
    return format("<%:%:%>", animated ? "a" : "", name, id);
}