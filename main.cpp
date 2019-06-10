#include <fstream>
#include <iostream>
#include <thread>

#define __DPP_DEBUG
#include "attachment.hpp"
#include "bot.hpp"
#include "context.hpp"
#include "discord.hpp"
#include "embedbuilder.hpp"
#include "events.hpp"
#include "member.hpp"
#include "user.hpp"
#include "role.hpp"
#include "utils.hpp"

int main() {
    std::ifstream file("token.txt");
    std::string token;
    std::getline(file, token);
    discord::Bot bot{ token, ">" };

    bot.register_callback<discord::events::ready>([&bot]() { std::cout << "Ready!" << std::endl
                                                                       << "Logged in as: " << bot.username << "#" << bot.discriminator
                                                                       << std::endl
                                                                       << "ID: " << bot.id << std::endl
                                                                       << "-----------------------------" << std::endl; });

    bot.register_command("test", [](discord::Context ctx) {
        std::string output = discord::format("%, these were your arguments: ", ctx.author->user->mention);
        for (auto const& each : ctx.arguments) {
            output += (each + " ");
        }
        ctx.channel->send(output)
            .get();
    });

    return bot.run();
}
