#pragma once
#include <string>
#include "color.hpp"
#include "discord.hpp"
#include "utility.hpp"


discord::User::User(snowflake id)
    : discord::Object(id) {
    auto user = discord::utils::get(discord::detail::bot_instance->users, [id](auto const& usr) {
        return usr->id == id;
    });
    if (user) {
        *this = *user;
    }
}

discord::User::User(nlohmann::json const j) {
    bot = j.contains("bot");
    discriminator = j["discriminator"];
    id = to_sf(j["id"]);
    avatar = format("https://cdn.discordapp.com/avatars/%/%.png", this->id, get_value(j, "avatar", ""));
    name = j["username"];
    mention = "<@" + std::to_string(id) + ">";
}

discord::Channel discord::User::create_dm() {
    return discord::Channel{
        send_request<request_method::Post>(nlohmann::json(
                                               { { "recipient_id", id } }),
                                           get_default_headers(),
                                           endpoint("/users/@me/channels"))
    };
}
