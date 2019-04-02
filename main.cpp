#include <iostream>
#include "sourcefiles/bot.h"
#include "curlpp/cURLpp.hpp"

#include "sourcefiles/WebsocketClient.h"

int main(){
    std::string token = "NTYyNjM1ODk5MzE0MzcyNjE4.XKNpig.QAiFqpcua1ojloNuVzEJtzJoqiU";
    curlpp::initialize(CURL_GLOBAL_ALL);
    Bot bot{ token, "." };
    if (!bot.authenticated){
        std::cout << "Failed to authenticate, error: " << bot.error_message << std::endl;
    } else {
        std::cout << "Logged in as " << bot.username << '#' << bot.discriminator << std::endl;
    }

    auto c = new WebsocketClient{};

    return 0;
}
