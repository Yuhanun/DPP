#pragma once
#include <string>
#include <color.hpp>
#include <discord.hpp>

namespace discord {
    class User : public discord::Object {
    public:
        User() : discord::Object() {}

        User(discord_id id) : discord::Object(id) {

        }
        bool bot;
        std::string name;
        std::string discriminator;
        std::string avatar;
        std::string avatar_url;
        discord::Color color;
        std::string display_name;
        std::string mention;
    };
};

