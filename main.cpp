#include <iostream>
#include "sourcefiles/bot.h"
#include "curlpp/cURLpp.hpp"

int main(){
    curlpp::initialize(CURL_GLOBAL_ALL);
    std::string token = "";
    Bot bot{ token, "." };
    if (!bot.authenticated){
        std::cout << "Failed to authenticate, error: " << bot.error_message << std::endl;
    }
    return 0;
}
