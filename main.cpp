#include <fstream>
#include <iostream>
#include <thread>

#define __DPP_DEBUG

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

    // auto l = [](discord::Message m) {
    //     std::stringstream s;
    //     s << "Embeds: " << m.embeds.size() << std::endl
    //       << "Attachments: " << m.attachments.size() << std::endl
    //       << "Content: " << m.content << std::endl
    //       << "Created at: " << m.timestamp << std::endl
    //       << "Edited at: " << m.edited_timestamp << std::endl;
    //     if (m.author) {
    //         s << "Author: " << m.author->name << "#" << m.author->discriminator << std::endl;
    //     }
    //     s << "-----------------------------" << std::endl;
    //     std::printf("%s", s.str().c_str());
    // };

    // bot.register_callback<discord::events::message_create>(l);
    // bot.register_callback<discord::events::message_update>(l);
    // bot.register_callback<discord::events::message_delete>(l);


    bot.register_command("test", [](discord::Context const& ctx) {
        std::vector<discord::Message> messages = { ctx.message };
        for (int i = 0; i < 5; i++) {
            messages.push_back(ctx.send("test"));
        }
        ctx.message.channel->bulk_delete(messages);
    });


    bot.register_command("help", [](discord::Context const& ctx) {
        ctx.send(discord::EmbedBuilder{}
                     .set_color(0xFFFFFF)
                     .set_description("Currently testing a different bot module for discord.\nhttps://github.com/yuhanun/dpp")
                     .set_author(
                         "Discord++",
                         "https://github.com/yuhanun/dpp",
                         ctx.author->avatar));
    });

    bot.run();
    return 0;
}