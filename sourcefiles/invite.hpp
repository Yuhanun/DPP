#pragma once

#include "bot.hpp"
#include "discord.hpp"
#include "utility.hpp"

discord::Invite::Invite(nlohmann::json const data) {
    code = data["code"];
    uses = get_value(data, "uses", 0);
    max_age = get_value(data, "max_age", 0);
    max_uses = get_value(data, "max_uses", 0);
    created_at = time_from_discord_string(get_value(data, "created_at", ""));

    snowflake temp_guild_id = to_sf(data["guild"]["id"]);
    auto guild = discord::utils::get(discord::detail::bot_instance->guilds, [temp_guild_id](auto const& g) {
        return g->id == temp_guild_id;
    });

    snowflake temp_channel_id = to_sf(data["channel"]["id"]);

    channel = discord::utils::get(guild->channels, [temp_channel_id](auto const& c) {
        return c.id == temp_channel_id;
    });
}

discord::Invite::Invite(std::string const& code)
    : code{ code } {
}


discord::Invite discord::Invite::get_invite() {
    return discord::Invite{
        send_request<request_method::Get>(
            nlohmann::json({ { "with_counts", true } }),
            get_default_headers(),
            get_invite_url())
    };
}
void discord::Invite::remove(){
    send_request<request_method::Delete>(
        nlohmann::json({}),
        get_default_headers(),
        get_delete_invite_url());
}

std::string discord::Invite::get_invite_url() {
    return format("%/invites/%", get_api(), code);
}

std::string discord::Invite::get_delete_invite_url() {
    return get_invite_url();
}