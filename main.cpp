#include <fstream>
#include <iostream>
#include <thread>

#include "activity.hpp"
#include "embedbuilder.hpp"
#include "message.hpp"
#include "sourcefiles/bot.hpp"
#include "utility.hpp"
#include "webhook.hpp"


int main() {
    std::ifstream file("token.txt");
    std::string token;
    std::getline(file, token);
    discord::Bot bot{ token, "." };

    bot.register_callback<discord::events::ready>([&bot]() {
        std::cout << "Ready!" << std::endl
                  << "Logged in as: " << bot.username << "#" << bot.discriminator
                  << std::endl
                  << "ID: " << bot.id << std::endl
                  << "-----------------------------" << std::endl;
    });

    auto l = [](discord::Message m) {
        std::stringstream s;
        s << "Embeds: " << m.embeds.size() << std::endl
          << "Attachments: " << m.attachments.size() << std::endl
          << "Content: " << m.content << std::endl
          << "Created at: " << m.timestamp << std::endl
          << "Edited at: " << m.edited_timestamp << std::endl;
        if (m.author) {
            s << "Author: " << m.author->name << "#" << m.author->discriminator << std::endl;
        }
        s << "-----------------------------" << std::endl;
        std::printf("%s", s.str().c_str());
    };

    bot.register_callback<discord::events::message_create>(l);
    bot.register_callback<discord::events::message_update>(l);
    bot.register_callback<discord::events::message_delete>(l);

    bot.register_command("hello", [](discord::Context const& ctx) {
        ctx.send(discord::format("Hello %, you are in a guild called % with ID %",
                                 ctx.message.author->mention,
                                 ctx.message.author->guild->name,
                                 ctx.message.author->guild->id));
    });

    bot.run();
    return 0;
}