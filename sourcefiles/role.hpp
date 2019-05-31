#pragma once
#include "color.hpp"
#include "discord.hpp"
#include "permissions.hpp"
#include "utility.hpp"

discord::Role::Role(snowflake id) {
    discord::utils::get(discord::detail::bot_instance->guilds, [id, this](auto const& g) {
        for (auto const& role : g->roles) {
            if (role.id == id) {
                *this = role;
                return true;
            }
        }
        return false;
    });
}

discord::Role::Role(nlohmann::json data, discord::Guild* g) {
    hoist = data["hoist"];
    managed = data["managed"];
    mentionable = data["mentionable"];

    name = data["name"];
    color = discord::Color(data["color"].get<int>());
    permissions = PermissionOverwrites(data["permissions"].get<int>(), 0, id, object_type::role);
    guild = std::make_shared<discord::Guild>(*g);
}

void discord::Role::edit_position(int _new_position) {
    send_request<request_method::Post>(
        nlohmann::json({ { "id", id },
                         { "position", _new_position } }),
        get_default_headers(),
        format("%/guilds/%/roles", get_api(), this->guild->id));
}
