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
        if (m.channel.id != 563703009394294794 || m.channel.id != 563794584778309642){
            return;
        }
        if (m.author.bot){
            std::cout << "A bot said something" << std::endl;
            return;
        }
        m.channel.send("Hello human! " + m.author.mention);
    });
    // bot.register_callback<EVENTS::READY>([&bot](){
    //     discord::Channel channel{ 563794584778309642 };
    //     auto message = channel.send(
    //         discord::EmbedBuilder()
    //                 .set_title("Hello!")
    //                 .set_color(discord::Color(0x00ff00))
    //                 .add_field("field_name_one", "field_value_one")
    //                 .add_field("field_name_two", "field_value_two")
    //                 .add_field("inline_field_name", "inline_field_value", true)
    //                 .add_field("inline_field_name_2", "inline_field_value_2", true)
    //                 .set_timestamp(discord::get_iso_datetime_now()),
    //                 "Content, in same message as an embed"
    //     );
    //     message.remove();
    // });
    bot.run();
    return 0;
}
