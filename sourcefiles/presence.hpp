#pragma once

#include "discord.hpp"
#include "utility.hpp"

discord::Presence::Presence(const nlohmann::json& j)
		: user(new discord::User{ discord::get_value(j, "user", nlohmann::json({})) }),
		  guild_id(new discord::Guild{ discord::detail::bot_instance->get_guild(to_sf(discord::get_value(j, "guild_id", "0"))) }),
		  status(discord::get_value(j, "status", "")),
		  client_status{ discord::get_value(j["client_status"], "desktop", ""),
		                 discord::get_value(j["client_status"], "mobile", ""),
		                 discord::get_value(j["client_status"], "web", "") } {
	if (j.contains("roles"))
		for (const auto& each : j["roles"])
			roles.emplace_back(new discord::Role{ each });
	if (j.contains("activities"))
		for (const auto& each : j["activities"])
			activities.emplace_back(new discord::Activity{ each });
}


