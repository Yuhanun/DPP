#include "context.hpp"
#include "message.hpp"
#include "channel.hpp"

discord::Context::Context(const discord::Bot* bot, discord::Message const& m, std::vector<std::string> const& vec, std::function<void(Context)> const& func, std::string const& func_name)
    : bot{ bot }, message{ m }, arguments{ vec }, command{ func }, command_name{ func_name }, author{ message.author }, channel{ message.channel } {
}