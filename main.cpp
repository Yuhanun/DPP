#include <iostream>
#include <thread>

#include "embedbuilder.hpp"
#include "message.hpp"
#include "sourcefiles/bot.hpp"
#include "utility.hpp"

using namespace nlohmann;

void do_smth() { return; }

int main() {
  const std::string token =
      "";
  discord::Bot bot{token, "."};
  bot.register_callback<EVENTS::READY>([&bot]() {
    std::cout << "Ready!" << std::endl;
    std::cout << "Logged in as: " << bot.username << "#" << bot.discriminator
              << std::endl;
    std::cout << "ID: " << bot.id << std::endl;
    std::cout << "-----------------------------" << std::endl;
  });

  bot.register_command(
      "nothing", [&bot](discord::Message &m, std::vector<std::string> &args) {
        m.channel.get_message(m.id).remove();
      });

  bot.register_command(
      "test", [&bot](discord::Message &m, std::vector<std::string> &args) {
        if (m.author.id != 553478921870508061) {
          m.channel.send("Only Mehodin can use this command");
          return;
        }
        m.channel.typing();
        m.pin();
        m.channel.send("Pinned!");
        m.channel.typing();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        m.unpin();
        m.channel.send("Unpinned!");
      });

  bot.run();
  return 0;
}