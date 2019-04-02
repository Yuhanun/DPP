#include <list>
#include <array>
#include <thread>
#include <sstream>

#include "curlpp/cURLpp.hpp"
#include "curlpp/Easy.hpp"
#include "curlpp/Options.hpp"

#include "nlohmann/json.hpp"

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>

#include "gatewayhandler.hpp"

using namespace nlohmann;
using namespace boost;

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class Bot {
public:
    Bot(const std::string& token, const std::string prefix, std::thread* t)
        : prefix{prefix}, token{token}, gateway_thread{t}
    {
        curlpp::Cleanup cleaner;
        curlpp::Easy request;
        request.setOpt(new curlpp::options::Url(auth_url));
        request.setOpt(new curlpp::options::HttpHeader(get_basic_header()));
        json j;
        std::stringstream s;
        s << request;
        j << s;
        authenticated = false;
        initialize_variables(j);
        gateway_auth();
        authenticated = true;
    }

    void send_message(long channel_id, std::string message_content){
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        request.setOpt(new curlpp::options::Url(get_channel_link(channel_id)));
        auto headers = get_basic_header();
        headers.push_back("Content-Type: application/json");
        request.setOpt(new curlpp::options::HttpHeader(headers));
    }

private:
    std::string get_channel_link(long id){
        return "http://discordapp.com/api/v6/channels/" + std::to_string(id) + "/messages";
    }

    void gateway_auth(){
        gateway_thread = new std::thread{ handle_gateway, get_gateway_url() };
    }

    std::string get_gateway_url(){
        auto url = "https://discordapp.com/api/v6/gateway/bot";
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        request.setOpt(new curlpp::options::Url(url));
        auto headers = get_basic_header();
        request.setOpt(new curlpp::options::HttpHeader(headers));
        std::stringstream s;
        s << request;
        json j;
        j << s;
        std::cout << j.dump(4) << std::endl;
        return j["url"];
    }

    bool initialize_variables(const json& j){
        std::cout << j.dump(4) << std::endl;
        if (j.contains("message")){
            error_message = j["message"];
            return false;
        }
        std::string temp_id = j["id"];
        std::string temp_discrim = j["discriminator"];
        id = std::stol(temp_id);
        discriminator = std::stoi(temp_discrim);

        flags = j["flags"];
        verified = j["verified"];
        mfa_enabled = j["mfa_enabled"];
        bot = j["bot"];
        username = j["username"];
        locale = j["locale"];
        avatar = j["avatar"].is_string() ? j["avatar"] : "Null";
        email = j["email"].is_string() ? j["email"] : "Null";
        return true;
    }

    std::list<std::string> get_basic_header() {
        return { "Authorization: Bot " + token };
    }


public:
    bool authenticated;
    std::string error_message;

    int flags;
    long id;
    int discriminator;
    bool bot;
    bool verified;
    bool mfa_enabled;
    std::string email;
    std::string avatar;
    std::string locale;
    std::string username;

    std::string prefix;
private:
    std::thread* gateway_thread;
    std::string token;
    std::string auth_url = "https://discordapp.com/api/v6/users/@me";
};
