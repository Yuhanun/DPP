#pragma once

#include <bot.hpp>
#include <discord.hpp>
#include <utility.hpp>

discord::Invite::Invite(std::string const& invite_str){
    auto data = json::parse(invite_str);
    uses = get_value(data, "uses", 0);
    max_age = get_value(data, "max_age", 0);
    max_uses = get_value(data, "max_uses", 0);

    code = get_value(data, "code", "");
    created_at = get_value(data, "created_at", "");

    snowflake temp_guild_id = std::stoul(data["guild"]["id"].get<std::string>());
    auto guild = discord::utils::get(discord::bot_instance->guilds, [temp_guild_id](std::unique_ptr<discord::Guild>& g){
        return g->id == temp_guild_id;
    });

    snowflake temp_channel_id = std::stoul(data["channel"]["id"].get<std::string>());
    for (auto const& c : guild->channels){
        if (c.id != temp_channel_id){
            continue;
        }
        channel = c;
    }
}