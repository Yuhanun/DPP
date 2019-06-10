#pragma once
#include "discord.hpp"

namespace discord {
    class Integration {
    public:
        Integration(nlohmann::json const);

        snowflake id;
        std::string name;
        std::string type;
        bool enabled;
        bool syncing;
        std::shared_ptr<discord::Role> role;
        int expire_behavior;
        int expire_grace_period;
        std::shared_ptr<discord::User> user;
        snowflake account_id;
        std::string account_name;
        datetime synced_at;
    };
} // namespace discord