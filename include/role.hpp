#pragma once
#include "color.hpp"
#include "discord.hpp"
#include "permissions.hpp"
#include "object.hpp"

namespace discord {
    class Role : public Object {
    public:
        Role() = default;
        Role(snowflake);
        Role(nlohmann::json, std::shared_ptr<discord::Guild>);
        Role& update(nlohmann::json const);

        pplx::task<void> edit_position(int);
        pplx::task<discord::Role> edit(std::string const&, PermissionOverwrites&, discord::Color, bool, bool);
        pplx::task<void> remove();

    public:
        bool hoist;
        bool managed;
        bool mentionable;
        std::shared_ptr<discord::Guild> guild;

        std::string name;

        Color color;
        PermissionOverwrites permissions;
    };
};  // namespace discord