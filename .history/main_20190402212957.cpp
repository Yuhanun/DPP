#include <iostream>
#include <thread>

#include "sourcefiles/bot.hpp"
#include "curlpp/cURLpp.hpp"

int main(){
    std::string token = "NTYyNjM1ODk5MzE0MzcyNjE4.XKNpig.QAiFqpcua1ojloNuVzEJtzJoqiU";
    curlpp::initialize(CURL_GLOBAL_ALL);
    std::thread* t;
    Bot bot{ token, ".", t };
    std::cout << "Logged in as " << bot.username << '#' << bot.discriminator << std::endl;
    while (!bot.authenticated){

    }
    std::cout << "Joining\n";
    t->join();
    std::cout << "Joined!";
    delete t;
    return 0;
}
