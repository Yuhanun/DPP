#include <iostream>
#include <thread>

#include "embedbuilder.hpp"
#include "message.hpp"
#include "sourcefiles/bot.hpp"
#include "utility.hpp"


int main() {
    const std::string token = "";
    discord::Bot bot{ token, "." };

    // bot.register_callback<discord::events::guild_create>([&bot](discord::Guild g) {
    //     std::cout << g.name << " -> " << g.id << std::endl;
    //     for (auto const& each : g.channels) {
    //         std::cout << each.name << " -> " << each.id << std::endl;
    //     }
    //     std::cout << "----------------------------------" << std::endl;
    // });

    bot.register_callback<discord::events::ready>([&bot]() {
        std::cout << "Ready!" << std::endl;
        std::cout << "Logged in as: " << bot.username << "#" << bot.discriminator
                  << std::endl;
        std::cout << "ID: " << bot.id << std::endl;
        std::cout << "-----------------------------" << std::endl;
    });

    bot.register_command("spam", [&bot](discord::Message& m, std::vector<std::string>& args) {
        while (true){
            m.channel.send("spam!");
        }
        std::cout << "Here" << std::endl;
    });

    bot.run();
    return 0;
}