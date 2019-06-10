#pragma once
#include "discord.hpp"
#include "object.hpp"
#include "user.hpp"

namespace discord {
    class Emoji : public Object {
    public:
        Emoji() = default;
        Emoji(nlohmann::json);
        operator std::string();

    public:
        bool managed;
        bool animated;
        bool is_custom;
        bool require_colons;

        snowflake id;
        std::string name;
        discord::User user;
        discord::Asset image;
        std::vector<std::shared_ptr<discord::Role>> roles;
    };
} // namespace discord