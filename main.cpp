#include "attachment.hpp"
#include "bot.hpp"
#include "context.hpp"
#include "embedbuilder.hpp"
#include "events.hpp"
#include "member.hpp"
#include "role.hpp"
#include "user.hpp"
#include "utils.hpp"
#include "emoji.hpp"
#include "message.hpp"

int main() {
    std::ifstream file("token.txt");
    std::string token;
    std::getline(file, token);
    discord::Bot bot{ token, ">" };

    bot.register_callback<discord::events::ready>([&bot]() {
        std::cout << "Ready!" << std::endl
                  << "Logged in as: " << bot.username << "#" << bot.discriminator
                  << std::endl
                  << "ID: " << bot.id << std::endl
                  << "-----------------------------" << std::endl;
    });

    bot.register_callback<discord::events::message_reaction_add>([](discord::Message message, discord::Emoji emoji, discord::User user) {
        std::cout << "User : " << user.name << std::endl;
    });

    bot.register_command("test", "A Test command that prints command arguments out", {"..."}, [](discord::Context ctx) {
        std::string output = discord::format("%, these were your arguments: ", ctx.user->mention);
        for (auto const& each : ctx.arguments) {
            output += (each + " ");
        }
        ctx.channel->send(output)
            .wait();
    }, {
        [](discord::Context ctx)->bool {
            if (ctx.arguments.size() >= 2) {
                std::cout << "MORE THAN" << std::endl;
                return true;
            } else {
                std::cout << "LESS THAN" << std::endl;
                ctx.channel->send("Use more than 2 arguments!");
                return false;
            }
        }
        // List of requirement methods, they must return `true` for your command to execute
    });

    bot.register_command("presence", "Changes presence to streaming and text to \"Shroud is streaming!\"", {}, [](discord::Context ctx) {
        ctx.bot->update_presence(
            discord::Activity{
                "Shroud is streaming!",
                discord::presence::activity::streaming,
                discord::presence::status::idle,
                false,
                "http://twitch.tv/shroud" });
    }, {});

    return bot.run();
}
