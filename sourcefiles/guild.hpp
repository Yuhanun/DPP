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
            this
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

void discord::Guild::edit(std::string const& name, std::string const& rg, int verif_level, int default_message_notif, int explicit_cont_filt, snowflake afk_chan_id, int afk_timeout, std::string const& icon, snowflake owner_id, std::string const& splash, snowflake system_channel_id) {
    nlohmann::json data{
        { "name", name }
    };
    if (rg != "") data["region"] = rg;
    if (verif_level != -1) data["verification_level"] = verif_level;
    if (default_message_notif != -1) data["default_message_notifications"] = default_message_notif;
    if (explicit_cont_filt != -1) data["explicit_content_filter"] = explicit_cont_filt;
    if (afk_chan_id != -1) data["afk_channel_id"] = afk_chan_id;
    if (afk_timeout != -1) data["afk_timeout"] = afk_timeout;
    if (icon != "") data["icon"] = icon;
    if (owner_id != -1) data["owner_id"] = owner_id;
    if (splash != "") data["splash"] = splash;
    if (system_channel_id != -1) data["system_channel_id"] = system_channel_id;

    send_request<request_method::Patch>(
        data,
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

discord::Channel discord::Guild::create_channel(std::string const& name, bool nsfw, int type, std::string const& topic, int bitrate, int user_limit, int rate_limit_per_user, int position, std::vector<discord::PermissionOverwrites> const& permission_overwrites, snowflake parent_id) {
    nlohmann::json data = nlohmann::json({ { "name", name },
                                           { "nsfw", nsfw },
                                           { "permission_overwrites", nlohmann::json::array() } });
    if (type != -1) data["type"] = type;
    if (topic != "") data["topic"] = topic;
    if (bitrate != -1) data["bitrate"] = bitrate;
    if (user_limit != -1) data["user_limit"] = user_limit;
    if (rate_limit_per_user != -1) data["rate_limit_per_user"];
    if (position != -1) data["position"] = position;

    for (auto const& each : permission_overwrites) {
        data["permission_overwrites"].push_back(each.to_json());
    }

    if (parent_id != -1) data["parent_id"] = parent_id;

    return discord::Channel{
        send_request<request_method::Post>(data, get_default_headers(), format("%/guilds/%/channels", get_api(), id))
    };
}


void discord::Guild::remove_emoji(discord::Emoji const& emote) {
    send_request<request_method::Delete>(
        nlohmann::json({}),
        get_default_headers(),
        get_delete_emoji_url(emote.id));
}

discord::Member discord::Guild::get_member(snowflake m_id) {
    auto req_data = send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), format("%/guilds/%/members/%", get_api(), this->id, m_id));
    return discord::Member{
        req_data,
        req_data["user"],
        this
    };
}

std::vector<discord::Member> discord::Guild::get_members(int limit, snowflake after) {
    return from_json_array<discord::Member>(
        send_request<request_method::Get>(
            nlohmann::json({ { "limit", limit }, { "after", after } }),
            get_default_headers(),
            format("%/guilds/%/members", get_api(), this->id)));
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