#pragma once
#include "discord.hpp"
#include "member.hpp"

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
        boost::posix_time::ptime created_at{ boost::local_time::not_a_date_time };
        discord::Member inviter;
        std::shared_ptr<discord::Channel> channel;

        pplx::task<discord::Invite> get_invite();
        pplx::task<void> remove();
    };
} // namespace discord