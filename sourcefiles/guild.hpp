#pragma once
#include <nlohmann/json.hpp>
#include "discord.hpp"

#include "utility.hpp"

#include "channel.hpp"
#include "emoji.hpp"
#include "member.hpp"
#include "role.hpp"

discord::Guild::Guild(snowflake id)
    : discord::Object(id) {
    auto g = discord::utils::get(discord::detail::bot_instance->guilds, [id](auto const& guild) {
        return guild->id == id;
    });
    if (g) {
        *this = *g;
    }
}

discord::Guild::Guild(nlohmann::json const guild)
    : splash{ get_value(guild, "splash", 0) },
      mfa_level{ get_value(guild, "mfa_level", 0) },
      afk_timeout{ get_value(guild, "afk_timeout", 0) },
      member_count{ get_value(guild, "member_count", 0) },
      verification_level{ get_value(guild, "verification_level", 0) },
      explicit_content_filter{ get_value(guild, "explicit_content_filter", 0) },
      large{ get_value(guild, "large", true) },
      unavailable{ get_value(guild, "unavailable", false) },
      id{ to_sf(guild["id"]) },
      application_id{ to_sf(get_value(guild, "application_id", "0")) },
      name{ get_value(guild, "name", "") },
      icon{ get_value(guild, "icon", "") },
      region{ get_value(guild, "region", "") },
      banner{ get_value(guild, "banner", "") },
      created_at{ time_from_discord_string(guild["joined_at"]) },
      vanity_url_code{ get_value(guild, "vanity_url_code", "") },
      roles{ from_json_array<discord::Role>(guild, "roles") },
      emojis{ from_json_array<discord::Emoji>(guild, "emojis") } {
    for (auto& each : guild["members"]) {
        discord::Member member{
            each,
            discord::User(each["user"]),
            discord::utils::get(discord::detail::bot_instance->guilds, [this](auto const& g) {
                return this->id == g->id;
            })
        };
        members.emplace_back(std::make_shared<discord::Member>(member));
        if (each["user"]["id"] == guild["owner_id"]) {
            owner = member;
        }
    }

    for (auto const& each : guild["channels"]) {
        channels.emplace_back(std::make_shared<discord::Channel>(each, id));
    }
}

std::vector<discord::Webhook> discord::Guild::get_webhooks() {
    return from_json_array<discord::Webhook>(
        send_request<request_method::Get>(nlohmann::json(),
                                          get_default_headers(),
                                          get_webhooks_url()));
}

void discord::Guild::leave() {
    send_request<request_method::Delete>(nlohmann::json({}), get_default_headers(), get_leave_url());
}

std::vector<discord::Emoji> discord::Guild::list_emojis() {
    return from_json_array<discord::Emoji>(
        send_request<request_method::Get>(
            nlohmann::json({}), get_default_headers(), get_list_guild_emojis_url()));
}

discord::Emoji discord::Guild::get_emoji(discord::Emoji const& e) {
    return discord::Emoji{
        send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), get_guild_emoji(e.id))
    };
}

discord::Emoji discord::Guild::edit_emoji(discord::Emoji const& emote, std::string name, std::vector<discord::Role> roles) {
    nlohmann::json data(
        { { "name", name },
          { "roles", {} } });
    for (auto const& each : roles) {
        data["roles"].push_back(each.id);
    }
    return discord::Emoji{
        send_request<request_method::Patch>(
            data,
            get_default_headers(),
            get_modify_guild_emoji_url(emote.id))
    };
}

void discord::Guild::edit(std::string const& name, std::string const& region, int verif_level, int default_message_notif, int explicit_cont_filt, snowflake afk_chan_id, int afk_timeout, std::string const& icon, snowflake owner_id, std::string const& splash, snowflake system_channel_id) {
    send_request<request_method::Patch>(
        nlohmann::json({ { "name", name },
                         { "region", region },
                         { "verification_level", verif_level },
                         { "default_message_notifications", default_message_notif },
                         { "explicit_content_filter", explicit_cont_filt },
                         { "afk_channel_id", afk_chan_id },
                         { "afk_timeout", afk_timeout },
                         { "icon", icon },
                         { "owner_id", owner_id },
                         { "splash", splash },
                         { "system_channel_id", system_channel_id } }),
        get_default_headers(),
        format("%/guilds/%", get_api(), id));
}

void discord::Guild::remove() {
    send_request<request_method::Delete>(nlohmann::json({}), get_default_headers(), format("%/guilds/%", get_api(), id));
}

std::vector<discord::Channel> discord::Guild::get_channels() {
    return from_json_array<discord::Channel>(
        send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), format("%/guilds/%/channels", get_api(), id)), id);
}

void discord::Guild::remove_emoji(discord::Emoji const& emote) {
    send_request<request_method::Delete>(
        nlohmann::json({}),
        get_default_headers(),
        get_delete_emoji_url(emote.id));
}


std::string discord::Guild::get_list_guild_emojis_url() {
    return format("%/guilds/%/emojis", get_api(), id);
}

std::string discord::Guild::get_guild_emoji(snowflake e_id) {
    return format("%/guilds/%/emojis/%", get_api(), id, e_id);
}

std::string discord::Guild::get_modify_guild_emoji_url(snowflake e_id) {
    return get_guild_emoji(e_id);
}

std::string discord::Guild::get_delete_emoji_url(snowflake e_id) {
    return get_guild_emoji(e_id);
}

std::string discord::Guild::get_webhooks_url() const {
    return format("%/guilds/%/webhooks", get_api(), id);
}

std::string discord::Guild::get_leave_url() {
    return format("%/users/@me/guilds/%", get_api(), id);
}