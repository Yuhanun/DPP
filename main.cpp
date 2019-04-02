#include <iostream>
#include "sourcefiles/bot.h"
#include "curlpp/cURLpp.hpp"

int main(){
    curlpp::initialize(CURL_GLOBAL_ALL);
    auto token = "NTU2NTA3NTIwNjEzNDgyNTM3.D3Qr2Q.CQl8JzCNKFEhAsQ96TGNVbLJ-q4";
    Bot bot = Bot(token);
    std::cout << bot.avatar << std::endl;
    std::cout << bot.id << std::endl;
    return 0;
}
