#pragma once
#include "discord.hpp"
#include <nlohmann/json.hpp>

#include "utility.hpp"

#include "channel.hpp"
#include "emoji.hpp"
#include "member.hpp"
#include "role.hpp"

discord::Guild::Guild(snowflake id)
    : discord::Object(id) {
}

discord::Guild::Guild(nlohmann::json const guild) {
    for (auto &each : guild["roles"]) {
        roles.emplace_back(discord::Role{ each });
    }

    for (auto &each : guild["members"]) {
        discord::Member member{ each, discord::User(each["user"]) };
        members.push_back(member);
        if (each["user"]["id"] == guild["owner_id"]) {
            owner = member;
        }
    }

    for (auto const &emoji : guild["emojis"]) {
        emojis.push_back(discord::Emoji{ emoji });
    }

    id = to_sf(guild["id"]);

    for (auto &channel : guild["channels"]) {
        discord::Channel c{ channel, id };
        channels.push_back(c);
    }

    splash = get_value(guild, "splash", 0);
    mfa_level = get_value(guild, "mfa_level", 0);
    afk_timeout = get_value(guild, "afk_timeout", 0);
    member_count = get_value(guild, "member_count", 0);
    verification_level = get_value(guild, "verification_level", 0);
    explicit_content_filter = get_value(guild, "explicit_content_filter", 0);

    large = get_value(guild, "large", true);
    unavailable = get_value(guild, "unavailable", false);

    application_id = to_sf(get_value(guild, "application_id", "0"));

        name = guild["name"];
    icon = get_value(guild, "icon", "");
    region = guild["region"];
    banner = get_value(guild, "banner", "");
    created_at = time_from_discord_string(guild["joined_at"].get<std::string>());
    vanity_url_code = get_value(guild, "vanity_url_code", "");
}
