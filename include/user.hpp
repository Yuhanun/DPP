#pragma once
#include "object.hpp"
#include "assets.hpp"

namespace discord {
    class User : public Object {
    public:
        User() = default;
        User(snowflake);
        User(nlohmann::json const);
        User& update(nlohmann::json const);

        pplx::task<discord::Channel> create_dm();

    public:
        bool bot;

        std::string name;

        std::string mention;
        discord::Asset avatar;
        std::string discriminator;
    };
};  // namespace discord