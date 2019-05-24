#pragma once
#include <discord.hpp>

template <typename... Tys>
discord::Message discord::Context::send(Tys&&... args) const {
    return this->message.channel->send(std::forward<Tys>(args)...);
};

discord::Context::Context(const discord::Bot* bot, discord::Message const& m, std::vector<std::string> const& vec, std::function<void(Context const&)> const& func, std::string const& func_name)
    : bot{ bot }, message{ m }, arguments{ vec }, command{ func }, command_name{ func_name }, author{ message.author }, channel{ message.channel } {
}