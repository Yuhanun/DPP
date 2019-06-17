#pragma once
#include "member.hpp"
#include "nlohmann/json.hpp"

namespace discord {

    class Invite {
    public:
        Invite() = default;
        Invite(nlohmann::json const);
        Invite(std::string const&);

        int uses;
        int max_age;
        int max_uses;
        bool temporary;

        std::string code;
        boost::posix_time::ptime created_at{};
        discord::Member inviter;
        std::shared_ptr<discord::Channel> channel;

        pplx::task<discord::Invite> get_invite();
        pplx::task<void> remove();
    };
} // namespace discord