#pragma once
#include <cstdint>
#include <memory>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "nlohmann/json.hpp"

namespace discord {

    class Role;
    class User;

    typedef int64_t snowflake;

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
        boost::posix_time::ptime synced_at;
    };
} // namespace discord