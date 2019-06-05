#pragma once

#include "discord.hpp"
#include "utility.hpp"

discord::Presence::Presence(const nlohmann::json j)
    : status(discord::get_value(j, "status", "")) {
    auto client_status_obj = get_value(j, "client_status", nlohmann::json({}));
    client_status = {
        get_value(client_status_obj, "desktop", ""),
        get_value(client_status_obj, "mobile", ""),
        get_value(client_status_obj, "web", "")
    };

    if (j.contains("game")) {
        if (!j["game"].is_null()) {
            game = discord::Activity{ j["game"] };
        }
    }

    if (j.contains("roles"))
        for (const auto& each : j["roles"])
            roles.emplace_back(each);

    if (j.contains("activities"))
        for (const auto& each : j["activities"])
            activities.emplace_back(each);
}

void discord::Presence::update(nlohmann::json const data) {
    if (data.contains("client_status")) {
        auto client_status_obj = data["client_status"];
        client_status = {
            get_value(client_status_obj, "desktop", ""),
            get_value(client_status_obj, "mobile", ""),
            get_value(client_status_obj, "web", "")
        };
    }

    update_object(data, "status", status);

    if (data.contains("game")) {
        if (!data["game"].is_null()) {
            game = discord::Activity{ data["game"] };
        }
    }

    auto g_id = to_sf(data["guild_id"]);
    auto guild = discord::utils::get(discord::detail::bot_instance->guilds, [=](auto const& g) { return g->id == g_id; });

    if (data.contains("roles")) {
        roles.clear();
        for (const auto& each : data["roles"]) {
            roles.emplace_back(each, guild);
        }
    }

    if (data.contains("activities")) {
        activities.clear();
        for (const auto& each : data["activities"]) {
            activities.emplace_back(each);
        }
    }
}
