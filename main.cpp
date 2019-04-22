#include <iostream>
#include <thread>

#include "activity.hpp"
#include "embedbuilder.hpp"
#include "message.hpp"
#include "sourcefiles/bot.hpp"
#include "utility.hpp"


int main() {

    discord::Bot bot{ token, "." };

    bot.register_callback<discord::events::ready>([&bot]() {
        std::cout << "Ready!" << std::endl;
        std::cout << "Logged in as: " << bot.username << "#" << bot.discriminator
                  << std::endl;
        std::cout << "ID: " << bot.id << std::endl;
        std::cout << "-----------------------------" << std::endl;
    });

    // bot.register_callback<discord::events::message_update>([&bot](discord::Message m) {
    //     std::cout << "Message with ID: " << m.id << std::endl
    //               << "Updated at " << m.edited_timestamp << std::endl
    //               << "New text: " << m.content << std::endl
    //               << "-----------------------------" << std::endl;
    // });

    // bot.register_command("test", [&bot](discord::Message& m, std::vector<std::string>& args) {
    //     for (auto const& each : m.mentions) {
    //         std::cout << each.name << std::endl;
    //     }
    // });

    bot.run();
    return 0;
}