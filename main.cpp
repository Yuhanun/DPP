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
    bot.register_callback<EVENTS::MESSAGE_CREATE>([&bot](discord::Message m){
        auto channel = discord::utils::get(bot.channels, [&](std::unique_ptr<discord::Channel>& c){
            return c->id == 563703009394294794;
        });
        if (channel){
            std::cout << channel->id << std::endl;
        } else {
            std::cout << "Couldn't find channel" << std::endl;
        }
    });
    bot.run();
    return 0;
}
