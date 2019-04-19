#pragma once
#include "cpr/cpr.h"
#include "nlohmann/json.hpp"

#include "discord.hpp"
#include "events.hpp"
#include "exceptions.hpp"
#include "function_types.hpp"
#include "gatewayhandler.hpp"
#include "guild.hpp"
#include "message.hpp"

discord::Bot::Bot(const std::string &token, const std::string prefix)
    : prefix{ prefix }, token{ token } {
    discord::detail::bot_instance = this;
}

discord::Message discord::Bot::send_message(snowflake channel_id, std::string message_content, bool tts) {
    json j = json({ { "content", message_content }, { "tts", tts } });
    auto response = send_request<request_method::Post>(j, get_default_headers(), get_channel_link(channel_id));
    return discord::Message::from_sent_message(response);
}

discord::Message discord::Bot::send_message(snowflake channel_id, json message_content, bool tts) {
    message_content["tts"] = tts;
    auto response = send_request<request_method::Post>(message_content, get_default_headers(), get_channel_link(channel_id));
    return discord::Message::from_sent_message(response);
}

void discord::Bot::on_incoming_packet(websocketpp::connection_hdl, client::message_ptr msg) {
    packet_handling.push_back(
        std::async(std::launch::async, [=]() {
            json j = json::parse(msg->get_payload());
            switch (j["op"].get<int>()) {
                case (10):
                    hello_packet = j;
                    con->send(get_identify_packet());
                    break;
                case (11):
                    heartbeat_acked = true;
                    break;
                default:
                    std::string event_name = j["t"].is_null() ? "" : j["t"];
                    handle_event(j, event_name);
                    break;
            }
        }));
    packet_counter++;
}

void discord::Bot::handle_gateway() {
    std::string hostname = "discord.gg";
    std::string uri = get_gateway_url();
    try {
        // c.set_access_channels(websocketpp::log::alevel::none)
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        c.set_error_channels(websocketpp::log::elevel::all);

        c.init_asio();

        c.set_message_handler(bind(&Bot::on_incoming_packet, this, ::_1, ::_2));
        c.set_tls_init_handler(bind(&on_tls_init, hostname.c_str(), ::_1));

        websocketpp::lib::error_code ec;
        con = c.get_connection(uri, ec);
        if (ec) {
            throw std::runtime_error(ec.message());
        }

        c.connect(con);

        c.run();

    } catch (websocketpp::exception const &e) {
        std::cout << e.what() << std::endl;
    }
}

void discord::Bot::run() {
    gateway_auth();
    await_events();
    gateway_thread.join();
    event_thread.join();
}

void discord::Bot::handle_event(json const j, std::string event_name) {
    const json data = j["d"];
    if (j.contains("s")) {
        last_sequence_data = j["s"].is_number() ? j["s"].get<int>() : -1;
    }

    if (event_name == "HELLO") {
    } else if (event_name == "READY") {
        this->session_id = data["session_id"].get<std::string>();
        ready = true;
        heartbeat_thread = std::thread{ &Bot::handle_heartbeat, this };
        initialize_variables(data.dump());
        func_holder.call<events::ready>();
    } else if (event_name == "RESUMED") {
    } else if (event_name == "INVALID_SESSION") {
    } else if (event_name == "CHANNEL_CREATE") {
    } else if (event_name == "CHANNEL_UPDATE") {
    } else if (event_name == "CHANNEL_DELETE") {
    } else if (event_name == "CHANNEL_PINS_UPDATE") {
    } else if (event_name == "GUILD_CREATE") {
        snowflake guild_id = std::stoul(data["id"].get<std::string>());
        for (auto const &member : data["members"]) {
            for (auto const &each : this->users) {
                if (*each == std::stoul(member["user"]["id"].get<std::string>())) {
                    break;
                }
            }
            users.emplace_back(
                std::make_unique<discord::User>(member["user"]));
        }

        auto guild = discord::Guild(j["d"]);
        guilds.push_back(std::make_unique<discord::Guild>(j["d"]));

        for (auto const &channel : data["channels"]) {
            channels.emplace_back(
                std::make_unique<discord::Channel>(channel, guild_id));
        }
        func_holder.call<events::guild_create>(guild);
    } else if (event_name == "GUILD_UPDATE") {
    } else if (event_name == "GUILD_DELETE") {
        snowflake to_remove = std::stoul(data["id"].get<std::string>());
        guilds.erase(
            std::remove_if(guilds.begin(), guilds.end(), [&to_remove](std::unique_ptr<discord::Guild> const &g) {
                return g->id == to_remove;
            }),
            guilds.end());
    } else if (event_name == "GUILD_BAN_ADD") {
    } else if (event_name == "GUILD_BAN_REMOVE") {
    } else if (event_name == "GUILD_EMOJIS_UPDATE") {
    } else if (event_name == "GUILD_INTEGRATIONS_UPDATE") {
    } else if (event_name == "GUILD_MEMBER_ADD") {
    } else if (event_name == "GUILD_MEMBER_REMOVE") {
    } else if (event_name == "GUILD_MEMBER_UPDATE") {
    } else if (event_name == "GUILD_MEMBERS_CHUNK") {
    } else if (event_name == "GUILD_ROLE_CREATE") {
    } else if (event_name == "GUILD_ROLE_UPDATE") {
    } else if (event_name == "GUILD_ROLE_DELETE") {
    } else if (event_name == "MESSAGE_CREATE") {
        auto message = Message::from_sent_message(data);
        if (!message.author.bot) {
            fire_commands(message);
        }
        func_holder.call<events::message_create>(message);
    } else if (event_name == "MESSAGE_UPDATE") {
    } else if (event_name == "MESSAGE_DELETE") {
    } else if (event_name == "MESSAGE_DELETE_BULK") {
    } else if (event_name == "MESSAGE_REACTION_ADD") {
    } else if (event_name == "MESSAGE_REACTION_REMOVE") {
    } else if (event_name == "MESSAGE_REACTION_REMOVE_ALL") {
    } else if (event_name == "PRESENCE_UPDATE") {
    } else if (event_name == "TYPING_START") {
    } else if (event_name == "USER_UPDATE") {
    } else if (event_name == "VOICE_STATE_UPDATE") {
    } else if (event_name == "VOICE_SERVER_UPDATE") {
    } else if (event_name == "WEBHOOKS_UPDATE") {
    }
}

std::string discord::Bot::get_identify_packet() {
    json obj = { { "op", 2 },
                 { "d",
                   { { "token", token },
                     { "properties",
                       { { "$os", "Linux" },
                         { "$browser", "DiscordPP" },
                         { "$device", "DiscordPP" } } },
                     { "compress", false },
                     { "large_threshold", 250 } } } };
    return obj.dump();
}

void discord::Bot::gateway_auth() {
    gateway_thread = std::thread{ &Bot::handle_gateway, this };
}

void discord::Bot::await_events() {
    event_thread = std::thread{ [&]() {
        while (true){
            if (!packet_handling.size()){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            packet_handling[0].wait();
            packet_handling.erase(packet_handling.begin());
    } } };
}

std::string discord::Bot::get_gateway_url() {
    auto r = send_request<request_method::Get>(json({}), get_basic_header(), format("%/gateway/bot", get_api()));
    if (!r.contains("url")) {
        throw discord::ImproperToken();
    }
    return r["url"];
}

void discord::Bot::register_command(std::string const &command_name, std::function<void(discord::Message &, std::vector<std::string> &)> function) {
    command_map[boost::to_lower_copy(command_name)] = function;
}

void discord::Bot::fire_commands(discord::Message &m) const {
    std::vector<std::string> argument_vec;
    boost::split(argument_vec, m.content, boost::is_any_of(" "));
    if (argument_vec.size() < 1) {
        return;
    }
    auto command_name = argument_vec[0].substr(1, argument_vec[0].size());
    if (command_map.find(command_name) == command_map.end()) {
        return;
    }
    argument_vec.erase(argument_vec.begin());
    command_map.at(command_name)(m, argument_vec);
}

void discord::Bot::initialize_variables(const std::string raw) {
    json j = json::parse(raw);
    auto user = j["user"];
    std::string temp_id = user["id"];
    std::string temp_discrim = user["discriminator"];
    id = std::stoul(temp_id);
    discriminator = std::stoi(temp_discrim);

    verified = user["verified"];
    mfa_enabled = user["mfa_enabled"];
    bot = user["bot"];
    username = user["username"];
    avatar = j["avatar"].is_string() ? j["avatar"] : "Null";
    email = j["email"].is_string() ? j["email"] : "Null";
}

cpr::Header discord::Bot::get_basic_header() {
    return cpr::Header{
        { "Authorization", format("Bot %", token) }
    };
}

void discord::Bot::handle_heartbeat() {
    while (true) {
        json data;
        if (last_sequence_data == -1) {
            data = json({ { "op", 1 }, { "d", nullptr } });
        } else {
            data = json({ { "op", 1 }, { "d", last_sequence_data } });
        }
        con->send(data.dump());
        heartbeat_acked = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(hello_packet["d"]["heartbeat_interval"].get<int>()));
        if (!heartbeat_acked) {
            data = json({ { "token", token },
                          { "session_id", session_id },
                          { "seq", last_sequence_data } });
            con->send(data.dump());
        }
    }
}

discord::Guild discord::Bot::create_guild(std::string const &name, std::string const &region, int const &verification_level, int const &default_message_notifications, int const &explicit_content_filter) {
    const json data = json({ { "name", name },
                             { "region", region },
                             { "icon", "" },
                             { "verification_level", verification_level },
                             { "default_message_notifications", default_message_notifications },
                             { "explicit_content_filter", explicit_content_filter },
                             { "roles", {} },
                             { "channels", {} } });
    return discord::Guild{
        send_request<request_method::Post>(data, get_default_headers(), get_create_guild_url())
    };
}

std::string discord::Bot::get_create_guild_url() {
    return format("%/guilds", get_api());
}