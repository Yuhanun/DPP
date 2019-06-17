#pragma once
#include "discord.hpp"
#include "object.hpp"
#include "presence.hpp"

namespace discord {

    class Member : public Object {
    public:
        Member() = default;
        Member(snowflake);
        Member(nlohmann::json const, std::shared_ptr<discord::Guild>);
        Member& update(nlohmann::json const);  // TODO

        pplx::task<void> edit(std::string const&, bool, bool, std::vector<discord::Role> const& = {}, snowflake = -1);
        pplx::task<void> add_role(discord::Role const&);
        pplx::task<void> remove_role(discord::Role const&);

        pplx::task<void> kick();
        pplx::task<void> ban(std::string const&, int);

    public:
        bool deaf;
        bool muted;
        discord::Presence presence;
        std::vector<std::shared_ptr<discord::Role>> roles;
        std::shared_ptr<discord::User> user;
        std::shared_ptr<discord::Guild> guild;

        std::string nick;
        boost::posix_time::ptime joined_at{ boost::local_time::not_a_date_time };
    };
} // namespace discord