#pragma once
#include <cstdint>
#include <memory>

#include "assets.hpp"
#include "object.hpp"

#include "nlohmann/json.hpp"
#include "pplx/pplxtasks.h"
#include "boost/date_time/posix_time/posix_time.hpp"

namespace discord {

    class Channel;
    
    typedef int64_t snowflake;

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
}  // namespace discord