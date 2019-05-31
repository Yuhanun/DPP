#pragma once
#include "discord.hpp"
#include "user.hpp"

discord::Member::Member(snowflake id)
    : discord::User(id) {
    for (auto const& guild : discord::detail::bot_instance->guilds) {
        for (auto const& mem : guild->members) {
            if (id == mem->id) {
                *this = *mem;
            }
        }
    }
}

discord::Member::Member(nlohmann::json const j, discord::User const& user, discord::Guild* g) {
    deaf = get_value(j, "deaf", false);
    muted = get_value(j, "mute", false);
    nick = get_value(j, "nick", "");
    joined_at = time_from_discord_string(get_value(j, "joined_at", ""));
    guild = std::make_shared<discord::Guild>(*g);
    id = user.id;
    bot = user.bot;
    name = user.name;
    avatar = user.avatar;
    mention = user.mention;
    discriminator = user.discriminator;
    if (!j.contains("roles")) {
        return;
    }
    for (auto const& role : j["roles"]) {
        roles.emplace_back(to_sf(role));
    }
}

void discord::Member::edit(std::string const& t_name, bool t_mute, bool t_deaf, std::vector<discord::Role> const& t_roles, snowflake channel_id) {
    nlohmann::json data({ { "name", t_name },
                          { "mute", t_mute },
                          { "deaf", t_deaf },
                          { "roles", nlohmann::json::array() } });

    for (auto const& each : t_roles) {
        data["roles"].push_back(each.id);
    }

    if (channel_id != -1) {
        if (channel_id == 0)
            data["channel_id"] = nullptr;
        else
            data["channel_id"] = channel_id;
    }

    send_request<request_method::Patch>(
        data, get_default_headers(), format("%/guilds/%/members/%", get_api(), this->guild->id, this->id));
}
