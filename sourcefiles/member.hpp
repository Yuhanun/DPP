#pragma once
#include "discord.hpp"
#include "user.hpp"

discord::Member::Member(snowflake id)
    : discord::User(id) {
}

discord::Member::Member(nlohmann::json const j, discord::User const& user, discord::Guild* g) {
    deaf = get_value(j, "deaf", false);
    muted = get_value(j, "mute", false);
    nick = get_value(j, "nick", "");
    joined_at = time_from_discord_string(get_value(j, "joined_at", ""));
    guild = g;
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