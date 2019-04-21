#pragma once
#include <discord.hpp>
#include <user.hpp>

discord::Member::Member(snowflake id)
    : discord::User(id) {
}

discord::Member::Member(nlohmann::json const j, discord::User const &user) {
    deaf = j["deaf"];
    muted = j["mute"];
    nick = get_value(j, "nick", "");
    joined_at = j["joined_at"];
    id = user.id;
    bot = user.bot;
    name = user.name;
    avatar = user.avatar;
    mention = user.mention;
    discriminator = user.discriminator;
    // roles = j["roles"];
}