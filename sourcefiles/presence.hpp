#pragma once

#include "discord.hpp"
#include "utility.hpp"

discord::Presence::Presence(const nlohmann::json& j)
		: user(new discord::User{ discord::get_value(j, "user", nlohmann::json({})) }),
		  guild_id(new discord::Guild{ discord::detail::bot_instance->get_guild(to_sf(discord::get_value(j, "guild_id", "0"))) }),
		  status(discord::get_value(j, "status", "")) {
	if (j.contains("client_status")) {
		auto client_status_obj = get_value(j, "client", nlohmann::json({}));
		client_status = {
			get_value(client_status_obj, "desktop", ""),
			get_value(client_status_obj, "mobile", ""),
			get_value(client_status_obj, "web", "")
		};
	}
	if (j.contains("roles"))
		for (const auto& each : j["roles"])
			roles.emplace_back(new discord::Role{ each });
	if (j.contains("activities"))
		for (const auto& each : j["activities"])
			activities.emplace_back(new discord::Activity{ each });
}


