#pragma once
#include <color.hpp>
#include <discord.hpp>
#include <string>
#include "utility.hpp"


discord::User::User(snowflake id)
    : discord::Object(id) {
}

discord::User::User(nlohmann::json const j) {
    // string: name, avatar, mention, discriminator
    // int id
    // bool bot
    bot = j.contains("bot");
    avatar = discord::get_value(j, "avatar", "");
    discriminator = j["discriminator"];
    id = std::stoul(j["id"].get<std::string>());
    name = j["username"];
    mention = "<@" + std::to_string(id) + ">";
}