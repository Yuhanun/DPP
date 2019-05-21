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
    auto g = discord::utils::get(discord::detail::bot_instance->guilds, [&id](auto const& guild) {
        return guild->id == id;
    });
    if (g){
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
      roles{ from_json_array<discord::Role>(guild["roles"]) },
      emojis{ from_json_array<discord::Emoji>(guild["emojis"]) },
      channels{ from_json_array<discord::Channel>(guild["channels"]) } {
      for (auto& each : guild["members"]) {
        discord::Member member{ each, discord::User(each["user"]), this };
        members.push_back(member);
        if (each["user"]["id"] == guild["owner_id"]) {
            owner = member;
        }
    }
}
