#pragma once

#include "bot.hpp"
#include "discord.hpp"
#include "utility.hpp"

discord::Invite::Invite(discord::Bot* bot_instance, nlohmann::json const data) {
    uses = get_value(data, "uses", 0);
    max_age = get_value(data, "max_age", 0);
    max_uses = get_value(data, "max_uses", 0);
    created_at = get_value(data, "created_at", "");

    snowflake temp_guild_id = to_sf(data["guild"]["id"]);
    auto guild = discord::utils::get(bot_instance->guilds, [temp_guild_id](std::unique_ptr<discord::Guild> &g) {
        return g->id == temp_guild_id;
    });

    snowflake temp_channel_id = to_sf(data["channel"]["id"]);
    for (auto const &c : guild->channels) {
        if (c.id != temp_channel_id) {
            continue;
        }
        channel = c;
    }
}