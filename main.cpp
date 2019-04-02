#include <iostream>
#include "sourcefiles/bot.h"
#include "curlpp/cURLpp.hpp"

int main(){
    curlpp::initialize(CURL_GLOBAL_ALL);
    std::string token = "";
    Bot bot{ token, "." };
    std::cout << bot.avatar << std::endl;
    std::cout << bot.id << std::endl;
    return 0;
}
