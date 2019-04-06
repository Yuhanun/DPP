#pragma once
#include "nlohmann/json.hpp"

#include "message.hpp"
#include "discord.hpp"
#include "guild.hpp"
#include "events.hpp"
#include "function_types.hpp"
#include "gatewayhandler.hpp"


discord::Bot::Bot(const std::string& token, const std::string prefix)
    : prefix{prefix}, token{token}
{
    curlpp::initialize(CURL_GLOBAL_ALL);
    discord::Channel::bot = this;
}


discord::Message discord::Bot::send_message(discord_id channel_id, std::string message_content){
    auto h = get_basic_header();
    h.push_back("Content-Type: application/json");
    h.push_back("User-Agent: DiscordPP (C++ discord library)");
    h.push_back("Connection: keep-alive");
    json j = json(
        {
            {"content", message_content},
            {"tts", false}
        }
    );

    json response = send_request(j, h, get_channel_link(channel_id));
    return discord::Message::from_sent_message(response.dump(), this);
}


void discord::Bot::write_to_file(std::string event_name, std::string data){
    struct stat buffer;   
    bool exists = (stat (event_name.c_str(), &buffer) == 0); 
    if (exists){
        return;
    }
    std::ofstream output_file(event_name, std::ios::out);
    output_file << data;
}

void discord::Bot::on_incoming_packet(websocketpp::connection_hdl, client::message_ptr msg) {
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


void discord::Bot::handle_gateway(){
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

void discord::Bot::run(){
    gateway_auth();
    gateway_thread.join();
}

void discord::Bot::handle_event(json& j, std::string event_name){
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
        initialize_variables(j["d"].dump());
        func_holder.call<EVENTS::READY>();
    } else if (event_name == "ERROR"){
        
    } else if (event_name == "GUILD_STATUS"){

    } else if (event_name == "GUILD_CREATE"){
        guilds.emplace_back(std::make_unique<discord::Guild>(j.dump()));
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
        auto message = Message::from_sent_message(j["d"].dump(), this);
        func_holder.call<EVENTS::MESSAGE_CREATE>(message);
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


std::string discord::Bot::get_identify_packet() {
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

void discord::Bot::gateway_auth(){
    gateway_thread = std::thread{ &Bot::handle_gateway, this };
}

std::string discord::Bot::get_gateway_url(){
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

void discord::Bot::initialize_variables(const std::string raw){
    json j = json::parse(raw);
    auto user = j["user"];
    std::string temp_id = user["id"];
    std::string temp_discrim = user["discriminator"];
    id = std::stol(temp_id);
    discriminator = std::stoi(temp_discrim);

    verified = user["verified"];
    mfa_enabled = user["mfa_enabled"];
    bot = user["bot"];
    username = user["username"];
    avatar = j["avatar"].is_string() ? j["avatar"] : "Null";
    email = j["email"].is_string() ? j["email"] : "Null";
}

std::list<std::string> discord::Bot::get_basic_header() {
    return { "Authorization: Bot " + token };
}

void discord::Bot::handle_heartbeat(){
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
