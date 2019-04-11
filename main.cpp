#include <iostream>
#include <thread>

#include "sourcefiles/bot.hpp"
#include "message.hpp"
#include "utility.hpp"
#include "embedbuilder.hpp"

int main(){
    const std::string token = "";
    discord::Bot bot{ token, "." };
    bot.register_callback<EVENTS::READY>([&bot](){
        std::cout << "Ready!" << std::endl;
        std::cout << "Logged in as: " << bot.username << "#" << bot.discriminator << std::endl;
        std::cout << "ID: " << bot.id << std::endl;
        std::cout << "-----------------------------" << std::endl;
    });

    bot.register_command("nothing", [&bot](discord::Message& m, std::vector<std::string>& args){
        m.channel.get_message(m.id).remove();
    });

    bot.register_command("delete", [&bot](discord::Message& m, std::vector<std::string>& args){
        if (m.author.id != 553478921870508061){
            m.channel.send("Only Mehodin can use this command");
            return;
        }
        auto perms = discord::PermissionOverwrites{270104678648250378, discord::PermissionOverwrites::member}
                             .add_permission("ADMINISTRATOR", false)
                             .add_permission("READ_MESSAGES", false);
        
        for (auto const& each : perms.allow_perms.ows){
            std::cout << each.first << " -> " << each.second << std::endl;
        }

        for (auto const& each : perms.deny_perms.ows){
            std::cout << each.first << " -> " << each.second << std::endl;
        }

        auto data = nlohmann::json({
            {"permission_overwrites", { perms.to_json() } }
        });
        std::cout << data.dump(4) << std::endl;
        m.channel.edit(data);
    });
    
    bot.run();
    return 0;
}
