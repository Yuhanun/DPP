#include <fstream>
#include <iostream>
#include <thread>

#include "activity.hpp"
#include "embedbuilder.hpp"
#include "message.hpp"
#include "sourcefiles/bot.hpp"
#include "utility.hpp"


int main() {
    std::ifstream file("token.txt");
    std::string token;
    std::getline(file, token);
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

    auto l = [&bot](discord::Message m) {
        std::cout << "Content: " << m.content << std::endl
                  << "Created at: " << m.timestamp << std::endl
                  << "Edited at: " << m.edited_timestamp << std::endl
                  << "Author: " << m.author.name << "#" << m.author.discriminator << std::endl
                  << "-----------------------------" << std::endl;
    };
    bot.register_callback<discord::events::message_create>(l);
    bot.register_callback<discord::events::message_update>(l);
    bot.run();
    return 0;
}