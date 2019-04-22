#include <iostream>
#include <thread>
#include <fstream>

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

    bot.register_command("test", [&bot](discord::Message& m, std::vector<std::string>& args) {
        discord::EmbedBuilder embed{};
        embed.set_color(0x00ff00).set_description("Description");
        auto msg = m.channel.send(embed);
        msg.channel.send(msg.embeds[0]);
    });

    bot.run();
    return 0;
}