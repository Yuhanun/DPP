#include <iostream>
#include <thread>

#include "embedbuilder.hpp"
#include "message.hpp"
#include "sourcefiles/bot.hpp"
#include "utility.hpp"

using namespace nlohmann;

int main() {
    const std::string token = "";
    discord::Bot bot{ token, "." };

    bot.register_callback<discord::events::guild_create>([&bot](discord::Guild g) {
        std::cout << g.name << " -> " << g.id << std::endl;
        for (auto const& each : g.channels) {
            std::cout << each.name << " -> " << each.id << std::endl;
        }
        std::cout << "----------------------------------" << std::endl;
    });

    bot.register_callback<discord::events::ready>([&bot]() {
        std::cout << "Ready!" << std::endl;
        std::cout << "Logged in as: " << bot.username << "#" << bot.discriminator
                  << std::endl;
        std::cout << "ID: " << bot.id << std::endl;
        std::cout << "-----------------------------" << std::endl;
    });

    bot.register_command(
        "nothing", [&bot](discord::Message& m, std::vector<std::string>& args) {
            m.channel.send("Test");
        });

    bot.register_command("delete", [&bot](discord::Message& m, std::vector<std::string>& args) {
        m.channel.send("Test").remove();
    });

    bot.run();
    return 0;
}