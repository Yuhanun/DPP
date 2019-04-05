#pragma once
#include <list>
#include <array>
#include <chrono>
#include <thread>
#include <sstream>
#include <memory>
#include <curl/curl.h>
#include <unordered_map>

#include "guild.hpp"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

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
            initialize_curl();
            curlpp::Cleanup cleaner;
            curlpp::Easy request;
            request.setOpt(new curlpp::options::Url(auth_url));
            request.setOpt(new curlpp::options::HttpHeader(get_basic_header()));
            std::stringstream s;
            s << request;
            json j = json::parse(s.str());
            initialize_variables(j);
        }

        void send_message(discord_id channel_id, std::string message_content){
            auto h = get_basic_header();
            h.push_back("Content-Type: application/json");
            h.push_back("User-Agent: DiscordPP (C++ discord library)");
            h.push_back("Accept-Encoding: gzip, deflate");
            h.push_back("Connection: keep-alive");
            json j = json(
                {
                    {"content", message_content},
                    {"tts", false}
                }
            );

            json response = send_request(j, h, get_channel_link(channel_id));
        }

        void handle_event(json& j, std::string event_name){
            std::cout << "Incoming event: " << event_name << "\n";
            const json data = j["d"];
            if (j.contains("s")){
                last_sequence_data = j["s"].is_number() ? j["s"].get<int>() : -1;
            } else {
                last_sequence_data = -1;
            }
            if (event_name == "READY"){
                this->session_id = j["d"]["session_id"].get<std::string>();
                ready = true;
                heartbeat_thread = std::thread{ &Bot::handle_heartbeat, this };
            } else if (event_name == "ERROR"){
                
            } else if (event_name == "GUILD_STATUS"){

            } else if (event_name == "GUILD_CREATE"){
                guilds.push_back(std::make_unique<discord::Guild>(j.dump()));
            } else if (event_name == "CHANNEL_CREATE"){

            } else if (event_name == "VOICE_CHANNEL_SELECT"){
                
            } else if (event_name == "VOICE_STATE_CREATE"){

            } else if (event_name == "VOICE_STATE_UPDATE"){

            } else if (event_name == "VOICE_STATE_DELETE"){

            } else if (event_name == "VOICE_SETTINGS_UPDATE"){

            } else if (event_name == "VOICE_CONNECTION_STATUS"){

            } else if (event_name == "SPEAKING_START"){

            } else if (event_name == "SPEAKING_STOP"){

            } else if (event_name == "MESSAGE_CREATE"){

            } else if (event_name == "MESSAGE_UPDATE"){

            } else if (event_name == "MESSAGE_DELETE"){

            } else if (event_name == "NOTIFICATION_CREATE"){

            } else if (event_name == "CAPTURE_SHORTCUT_CHANGE"){

            } else if (event_name == "ACTIVITY_JOIN"){

            } else if (event_name == "ACTIVITY_SPECTATE"){

            } else if (event_name == "ACTIVITY_JOIN_REQUEST"){

            }
            // fire_events(event_name);

        }



        void on_incoming_packet(websocketpp::connection_hdl, client::message_ptr msg) {
            json j = json::parse(msg->get_payload());
            int opcode = j["op"];
            if (opcode == 11){
                c.get_alog().write(websocketpp::log::alevel::app, "Incoming heartbeat ACK");
                heartbeat_acked = true;
                return;
            }
            std::string event_name = j["t"].is_null() ? "" : j["t"];
            if (opcode == 10){
                hello_packet = j;
                con->send(get_identify_packet());
            } else {
                handle_event(j, event_name);
            }
            packet_counter++;
        }

        void initialize_curl(){
            curlpp::initialize(CURL_GLOBAL_ALL);
        }

        void handle_gateway(){
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
            return "https://discordapp.com/api/v6/channels/" + std::to_string(id) + "/messages";
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

        json send_request(const json& j, const std::list<std::string>& h, const std::string url){
            curlpp::Cleanup clean;
            curlpp::Easy r;
            r.setOpt(new curlpp::options::Url(url));
            r.setOpt(new curlpp::options::HttpHeader(h));
            r.setOpt(new curlpp::options::PostFields(j.dump()));
            r.setOpt(new curlpp::options::PostFieldSize(j.dump().size()));
            std::stringstream response_stream;
            response_stream << r;
            return json::parse(response_stream.str());
        }

        void handle_heartbeat(){
            while (true){
                json data;
                if (last_sequence_data == -1){
                    data = json({
                        {"op", 1},
                        {"d", nullptr}
                    });
                } else {
                    data = json({
                        {"op", 1},
                        {"d", nullptr}
                    });
                }
                c.get_alog().write(websocketpp::log::alevel::app, "Sending heartbeat packet!");
                con->send(data.dump());
                c.get_alog().write(websocketpp::log::alevel::app, "Sent heartbeat packet!");
                heartbeat_acked = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(hello_packet["d"]["heartbeat_interval"].get<int>()));
                if (heartbeat_acked){
                    c.get_alog().write(websocketpp::log::alevel::app, "Recieved heartbeat ack in last round, so continuing");
                }
            }
        }


    public:
        bool authenticated;
        std::string error_message;

        long id;

        int flags;
        int discriminator;

        bool bot;
        bool ready = false;
        bool verified;
        bool mfa_enabled;

        std::string email;
        std::string avatar;
        std::string locale;
        std::string username;
        std::string prefix;

        std::vector<std::shared_ptr<discord::Guild>> guilds;

    private:
        bool heartbeat_acked;
        json hello_packet;

        std::string session_id;
        std::string token;
        std::string auth_url = "https://discordapp.com/api/v6/users/@me";
        
        long long packet_counter;
        int last_sequence_data;

        client c;
        client::connection_ptr con;

        std::unordered_map<std::string, void(*)> event_map;
        std::thread gateway_thread;
        std::thread heartbeat_thread;
    };
}; //namespace discord