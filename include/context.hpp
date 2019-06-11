#pragma once
#include <discord.hpp>
#include <memory>
#include <string>
#include <vector>
#include "channel.hpp"
#include "message.hpp"

namespace discord {
    struct Context {
        Context(discord::Bot*, discord::Message, std::vector<std::string> const&, std::function<void(Context)>, std::string const&);
        discord::Bot* bot;
        discord::Message message;
        std::vector<std::string> arguments;
        std::function<void(Context)> command;
        std::string command_name;
        std::shared_ptr<discord::Member> author;
        std::shared_ptr<discord::Channel> channel;

        template <typename... Tys>
        pplx::task<discord::Message> send(Tys&&... args) {
            return message.channel->send(std::forward<Tys>(args)...);
        };
    };
} // namespace discord
