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

    bot.register_command("test", [&bot](discord::Message const& m, std::vector<std::string>& args) {
        for (auto const& guild : bot.guilds) {
            if (guild->name != "random_name") {
                continue;
            }
            for (auto const& channel : guild->channels) {
                if (channel.type == discord::channel_type::TextChannel) {
                    try {
                        std::cout << channel.create_invite().code << std::endl;
                        auto k = m.channel.send(channel.create_invite().code);
                        if (!k.sent) {
                            std::cout << k.error << std::endl;
                        }
                    } catch (discord::UnknownChannel) {
                        std::cout << channel.name << " -> " << channel.id << std::endl;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
            }
        }
    });

    bot.run();
    return 0;
}