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
    if (j.contains("avatar")) {
        if (j["avatar"].is_null()) {
            avatar = { "", default_user_avatar, to_sf(discriminator) };
        } else {
            std::string av_hash = j["avatar"];
            avatar = { j["avatar"], user_avatar, av_hash[0] == 'a' && av_hash[1] == '_', id };
        }
    }
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
