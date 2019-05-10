#pragma once
#include "color.hpp"
#include "discord.hpp"
#include "permissions.hpp"
#include "utility.hpp"

discord::Role::Role(discord::Bot* bot_instance, snowflake id) {
    for (auto const& guild : bot_instance->guilds) {
        for (auto const& role : guild->roles) {
            if (role.id != id) {
                continue;
            }
            *this = role;
            return;
        }
    }
}

discord::Role::Role(nlohmann::json data) {
    hoist = data["hoist"];
    managed = data["managed"];
    mentionable = data["mentionable"];

    name = data["name"];
    color = discord::Color(data["color"].get<int>());
    permissions = PermissionOverwrites(data["permissions"].get<int>(), 0, id, object_type::role);
}