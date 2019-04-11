#include <iostream>
#include <thread>

#include "sourcefiles/bot.hpp"
#include "message.hpp"
#include "utility.hpp"
#include "embedbuilder.hpp"

int main(){
    std::string token = "";
    discord::Bot bot{ token, "." };
    bot.register_callback<EVENTS::READY>([&bot](){
        std::cout << "Ready!" << std::endl;
        std::cout << "Logged in as: " << bot.username << "#" << bot.discriminator << std::endl;
        std::cout << "ID: " << bot.id << std::endl;
        std::cout << "-----------------------------" << std::endl;
    });

    bot.register_command("prune", [&bot](discord::Message& m, std::vector<std::string>& args){
        if (m.channel.id == 562636135428521986){
            auto messages = m.channel.get_messages(std::stoi(args[0]));
            m.channel.bulk_delete(messages);
            m.channel.send(discord::format("Deleted % messages ;)", args[0]));
        }
    });

    bot.register_command("spam", [&bot](discord::Message& m, std::vector<std::string>& args){
        if (m.author.id != 553478921870508061){
            m.channel.send("Only Mehodin can use this command");
            return;
        }
        for (int i = 0; i < std::stoi(args[0]); i++){
            m.channel.send("\U0001f602");
        }
    });

    bot.register_callback<EVENTS::MESSAGE_CREATE>([&bot](discord::Message m){

    });
    bot.run();
    return 0;
}
