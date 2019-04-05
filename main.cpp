#include <iostream>
#include <thread>

#include "sourcefiles/bot.hpp"
#include "message.hpp"

int main(){
    std::string token = "NTYyNjM1ODk5MzE0MzcyNjE4.XKNpig.QAiFqpcua1ojloNuVzEJtzJoqiU";
    discord::Bot bot{ token, "." };
    bot.register_callback<EVENTS::READY>([&bot](){
        std::cout << "Ready!" << std::endl;
        std::cout << "Logged in as: " << bot.username << "#" << bot.discriminator << std::endl;
        std::cout << "ID: " << bot.id << std::endl;
        std::cout << "-----------------------------" << std::endl;
    });
    bot.register_callback<EVENTS::READY>([&bot](){
        discord::Message message = bot.send_message(563703009394294794, "Events::Ready, second callback.");
        message.remove();
    });
    bot.run();
    return 0;
}
