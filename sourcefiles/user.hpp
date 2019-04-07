#pragma once
#include <string>
#include <color.hpp>
#include <discord.hpp>
#include "utility.hpp"


discord::User::User(discord_id id) : discord::Object(id) {

}

discord::User::User(std::string const& on_guild_create){
    // string: name, avatar, mention, discriminator
    // int id
    // bool bot

    json j = json::parse(on_guild_create);
    bot = j.contains("bot");
    avatar = discord::get_value(j, "avatar", "");
    discriminator = j["discriminator"];
    id = std::stoul(j["id"].get<std::string>());
    name = j["username"];
    mention = "<@" + std::to_string(id) + ">";
}