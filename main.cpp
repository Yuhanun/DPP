#include <fstream>
#include <iostream>
#include <thread>

#define __DPP_DEBUG

#include "discord.hpp"
#include "bot.hpp"
#include "context.hpp"
#include "events.hpp"

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

    bot.register_command("test", [](discord::Context const& ctx) mutable {
        while (true) {
            ctx.send("Test");
        }
    });

    return bot.run();
}
