#pragma once
#include "guild.hpp"
#include "channel.hpp"
#include "user.hpp"

namespace discord {
    class Webhook : public Object {
    public:
        Webhook() = default;
        Webhook(snowflake);
        Webhook(snowflake, std::string const&);
        Webhook(nlohmann::json const);

        snowflake id;
        discord::Guild guild;
        discord::Channel channel;
        std::optional<discord::User> user;
        std::string name;
        discord::Asset avatar;
        std::string token;

        // TODO: avatar edit for both
        pplx::task<discord::Webhook> edit(std::string const& = "", snowflake = 0);
        pplx::task<discord::Webhook> edit(std::string const& = "");

        pplx::task<void> remove();

        pplx::task<discord::Message> send(std::string const&, bool = false, std::string const& = "", std::string const& = "");
        pplx::task<discord::Message> send(std::vector<EmbedBuilder> const&, bool = false, std::string const& = "", std::string const& = "", std::string const& = "");

        pplx::task<void> execute_slack(bool, nlohmann::json const);
        pplx::task<void> execute_github(bool, nlohmann::json const);
    };
} // namespace discord