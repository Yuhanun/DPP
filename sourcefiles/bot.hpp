#pragma once
#include <list>
#include <array>
#include <thread>
#include <sstream>
#include <unordered_map>

#include "guild.hpp"

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

namespace discord {

    using namespace nlohmann;
    using namespace boost;

    typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
    typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;
    using websocketpp::lib::bind;

    class Bot {
    public:
        Bot(const std::string& token, const std::string prefix)
            : prefix{prefix}, token{token}
        {
            curlpp::Cleanup cleaner;
            curlpp::Easy request;
            request.setOpt(new curlpp::options::Url(auth_url));
            request.setOpt(new curlpp::options::HttpHeader(get_basic_header()));
            std::stringstream s;
            s << request;
            json j = json::parse(s.str());
            initialize_variables(j);
        }

        void send_message(long channel_id, std::string message_content){
            curlpp::Cleanup cleaner;
            curlpp::Easy request;

            request.setOpt(new curlpp::options::Url(get_channel_link(channel_id)));
            auto headers = get_basic_header();
            headers.push_back("Content-Type: application/json");
            request.setOpt(new curlpp::options::HttpHeader(headers));
        }

        void on_incoming_packet(websocketpp::connection_hdl, client::message_ptr msg) {
            auto k = msg->get_payload();
            json j = json::parse(msg->get_payload());
            int opcode = j["op"];
            std::string event_name = j["t"].is_null() ? "" : j["t"];
            if (opcode == 10){
                hello_packet = json::parse(msg->get_payload());
                con->send(get_identify_packet());
            } else {
                std::cout << event_name << std::endl;
                if (event_name == "GUILD_CREATE"){
                    guilds.push_back(new discord::Guild{j.dump()});
                }
                // handle_command();
            }
            packet_counter++;
        }

        void handle_gateway(){
            client c;
            std::string hostname = "discord.gg";
            std::string uri = get_gateway_url();
            try {
                c.set_access_channels(websocketpp::log::alevel::all);
                c.clear_access_channels(websocketpp::log::alevel::frame_payload);
                c.set_error_channels(websocketpp::log::elevel::all);

                c.init_asio();

                c.set_message_handler(bind(&Bot::on_incoming_packet, this, ::_1, ::_2));
                c.set_tls_init_handler(bind(&on_tls_init, hostname.c_str(), ::_1));

                websocketpp::lib::error_code ec;
                con = c.get_connection(uri, ec);
                if (ec) {
                    std::cout << "could not create connection because: " << ec.message() << std::endl;
                    return;
                }

                c.connect(con);

                c.get_alog().write(websocketpp::log::alevel::app, "Connecting to " + uri);
                
                c.run();

            } catch (websocketpp::exception const & e) {
                std::cout << e.what() << std::endl;
            }
        }
        
        void run(){
            gateway_auth();
            gateway_thread.join();
        }

    private:
        std::string get_channel_link(long id){
            return "http://discordapp.com/api/v6/channels/" + std::to_string(id) + "/messages";
        }

        std::string get_identify_packet() {
            json obj = {
                { "op", 2 },
                {
                    "d",
                    {
                        { "token", token },
                        { "properties",
                            {
                                { "$os", "Linux" },
                                { "$browser", "DiscordPP" },
                                { "$device", "DiscordPP" }
                            }
                        },
                        { "compress", false },
                        { "large_threshold", 250 }
                    }
                }
            };
            return obj.dump();
        }

        void gateway_auth(){
            gateway_thread = std::thread{ &Bot::handle_gateway, this };
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
            json j = json::parse(s.str());
            return j["url"];
        }

        bool initialize_variables(const json& j){
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

        long id;

        int flags;
        int discriminator;

        bool bot;
        bool verified;
        bool mfa_enabled;

        std::string email;
        std::string avatar;
        std::string locale;
        std::string username;
        std::string prefix;

        std::vector<discord::Guild*> guilds;

    private:
        json hello_packet;

        std::string token;
        std::string auth_url = "https://discordapp.com/api/v6/users/@me";
        
        long long packet_counter;

        client::connection_ptr con;
        
        std::unordered_map<std::string, void(*)> event_map;
        std::thread gateway_thread;
    };
}; //namespace discord