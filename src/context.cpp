#include "context.hpp"

discord::Context::Context(discord::Bot* bot, discord::Message m, std::vector<std::string> const& vec, std::function<void(Context)> func, std::string const& func_name)
    : bot{ bot }, message{ m }, arguments{ vec }, command{ func }, command_name{ func_name }, author{ message.author }, channel{ message.channel } {
    /**
     * @brief Constructor that's internally used for constructing a discord::Context
     * 
     * Should not be used by the user himself, unnessecary.
     */
}