#include <iostream>
#include <thread>

#include "sourcefiles/bot.hpp"
#include "message.hpp"
#include "utility.hpp"

int main(){
    std::cout << discord::get_iso_datetime_now() << std::endl;
    std::cout << discord::get_iso_from_unix(500) << std::endl;
    std::string token = "";
    discord::Bot bot{ token, "." };
    bot.register_callback<EVENTS::READY>([&bot](){
        std::cout << "Ready!" << std::endl;
        std::cout << "Logged in as: " << bot.username << "#" << bot.discriminator << std::endl;
        std::cout << "ID: " << bot.id << std::endl;
        std::cout << "-----------------------------" << std::endl;
    });
    bot.register_callback<EVENTS::READY>([&bot](){
        discord::Channel channel{ 563794584778309642 };
        auto message = channel.send("Deleting this message after 0.5 seconds");
        message.remove();
    });
    bot.run();
    return 0;
}
