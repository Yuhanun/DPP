#include <iostream>
#include <thread>

#include "sourcefiles/bot.hpp"
#include "curlpp/cURLpp.hpp"

int main(){
    std::string token = "NTYyNjM1ODk5MzE0MzcyNjE4.XKNpig.QAiFqpcua1ojloNuVzEJtzJoqiU";
    curlpp::initialize(CURL_GLOBAL_ALL);
    discord::Bot bot{ token, "." };
    std::cout << "Logged in as " << bot.username << '#' << bot.discriminator << std::endl;
    bot.run();
    return 0;
}
