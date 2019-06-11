#include "context.hpp"
#include "message.hpp"
#include "channel.hpp"

discord::Context::Context(discord::Bot* bot, discord::Message m, std::vector<std::string> const& vec, std::function<void(Context)> func, std::string const& func_name)
    : bot{ bot }, message{ m }, arguments{ vec }, command{ func }, command_name{ func_name }, author{ message.author }, channel{ message.channel } {
}