#pragma once
#include "discord.hpp"
#include "utility.hpp"


discord::Integration::Integration(nlohmann::json const data)
    : id{ to_sf(get_value(data, "id", "0")) },
      name{ get_value(data, "name", "") },
      type{ get_value(data, "type", "") },
      enabled{ get_value(data, "enabled", false) },
      syncing{ get_value(data, "syncing", false) },
      expire_behavior{ get_value(data, "expire_behavior", 0) },
      expire_grace_period{ get_value(data, "expire_grace_period", 0) },
      user{ std::make_shared<discord::User>(discord::User{ data["user"] }) },
      account_id{ to_sf(get_value(get_value(data, "account", nlohmann::json({})), "id", "0")) },
      account_name{ get_value(get_value(data, "account", nlohmann::json({})), "name", "") },
      synced_at{ time_from_discord_string(get_value(data, "synced_at", "")) } {
    auto role_id = to_sf(get_value(data, "role_id", "0"));
    for (auto const& guild : discord::detail::bot_instance->guilds) {
        for (auto const& role : guild->roles) {
            if (role.id == role_id) {
                this->role = std::make_shared<discord::Role>(role);
                break;
            }
        }
    }
}