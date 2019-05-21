#pragma once
#include <string>
#include "color.hpp"
#include "discord.hpp"
#include "utility.hpp"


discord::User::User(snowflake id)
    : discord::Object(id) {
    auto user = discord::utils::get(discord::detail::bot_instance->users, [&id](auto const& usr) {
        return usr->id == id;
    });
    if (user) {
        *this = *user;
    }
}

discord::User::User(nlohmann::json const j) {
    bot = j.contains("bot");
    avatar = get_value(j, "avatar", "");
    discriminator = j["discriminator"];
    id = to_sf(j["id"]);
    name = j["username"];
    mention = "<@" + std::to_string(id) + ">";
}