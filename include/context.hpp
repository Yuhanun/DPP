#pragma once
#include <discord.hpp>
#include <memory>
#include <string>
#include <vector>
#include "channel.hpp"
#include "message.hpp"

namespace discord {
    struct Context {
        /**
         * @brief Struct used for passing to commands
         * 
         * ```cpp
         *      bot.register_event("hello", [](discord::Context ctx) {
         *          ctx.channel->send("hello!").wait();
         *      });
         * ```
         */
        Context(discord::Bot*, discord::Message, std::vector<std::string> const&, std::function<void(Context)>, std::string const&);
        discord::Bot* bot; /**< A raw pointer to your bot instance */
        discord::Message message; /**< A discord::Message object, of the message that triggered this command */
        std::vector<std::string> arguments; /**< Everything after prefix + command name, separated by spaces */
        std::function<void(Context)> command; /**< Function that this Context object is being passed to */
        std::string command_name; /**< Name of the invoked command */
        std::shared_ptr<discord::Member> author; /**< Author of the message, shortcut for ctx.message.author */
        std::shared_ptr<discord::Channel> channel; /**< Channel to which the message was sent, shortcut for ctx.message.channel */

        template <typename... Tys>
        pplx::task<discord::Message> send(Tys&&... args) {
            /**
             * @brief Simply calls message.channel->send with the same parameters
             * 
             * ```cpp
             *      ctx.send("Hello!").wait();
             * ```
             * 
             * @param[in] Tys&&...args Argument pack that should contain all arguments you want to forward to message.channel->send()
             */
            return message.channel->send(std::forward<Tys>(args)...);
        };
    };
} // namespace discord
