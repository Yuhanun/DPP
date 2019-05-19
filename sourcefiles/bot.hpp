#pragma once
#include "context.hpp"
#include "cpr/cpr.h"
#include "discord.hpp"
#include "events.hpp"
#include "exceptions.hpp"
#include "gatewayhandler.hpp"
#include "guild.hpp"
#include "message.hpp"
#include "nlohmann/json.hpp"

discord::Bot::Bot(const std::string &token, const std::string prefix, std::size_t message_cache_count)
    : ready{ false }, token{ token }, prefix{ prefix }, message_cache_count{ message_cache_count } {
    discord::detail::bot_instance = this;
}

discord::Message discord::Bot::send_message(snowflake channel_id, std::string message_content, bool tts) {
    nlohmann::json j = nlohmann::json({ { "content", message_content }, { "tts", tts } });
    auto response = send_request<request_method::Post>(j, get_default_headers(), get_channel_link(channel_id));
    return discord::Message::from_sent_message(response);
}

discord::Message discord::Bot::send_message(snowflake channel_id, nlohmann::json message_content, bool tts) {
    message_content["tts"] = tts;
    auto response = send_request<request_method::Post>(message_content, get_default_headers(), get_channel_link(channel_id));
    return discord::Message::from_sent_message(response);
}

void discord::Bot::on_incoming_packet(const websocketpp::connection_hdl &, const client::message_ptr &msg) {
    nlohmann::json j = nlohmann::json::parse(msg->get_payload());
    switch (j["op"].get<int>()) {
        case (10):
            hello_packet = j;
            con->send(get_identify_packet());
            break;
        case (11):
            heartbeat_acked = true;
            break;
        default:
            std::string event_name = get_value(j, "t", "");
            handle_event(j, event_name);
            break;
    }
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

        c.set_message_handler(std::bind(&Bot::on_incoming_packet, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        c.set_tls_init_handler(std::bind(&on_tls_init, hostname.c_str(), websocketpp::lib::placeholders::_1));

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

void discord::Bot::handle_event(nlohmann::json const j, std::string event_name) {
    const nlohmann::json data = j["d"];
    last_sequence_data = j["s"].is_number() && j.contains("s") ? j["s"].get<int>() : -1;
    bool found = false;

    if (event_name == "HELLO") {
        func_holder.call<events::hello>(packet_handling, true);
    } else if (event_name == "READY") {
        this->session_id = data["session_id"].get<std::string>();
        heartbeat_thread = std::thread{ &Bot::handle_heartbeat, this };
        initialize_variables(data.dump());
        ready_packet = data;
    } else if (event_name == "RESUMED") {
        func_holder.call<events::resumed>(packet_handling, true);
    } else if (event_name == "INVALID_SESSION") {
        func_holder.call<events::invalid_session>(packet_handling, true);
    } else if (event_name == "CHANNEL_CREATE") {
        channel_create_event(j);
    } else if (event_name == "CHANNEL_UPDATE") {
        channel_update_event(j);
    } else if (event_name == "CHANNEL_DELETE") {
        channel_delete_event(j);
    } else if (event_name == "CHANNEL_PINS_UPDATE") {
        func_holder.call<events::channel_pins_update>(packet_handling, true, Channel{ to_sf(data["channel_id"]) });
    } else if (event_name == "GUILD_CREATE") {
        guild_create_event(j);
    } else if (event_name == "GUILD_UPDATE") {
        snowflake new_guild = to_sf(data["id"]);
        Guild g;
        for (auto &each : guilds) {
            if (new_guild == each->id) {
                g = *(std::make_unique<Guild>(data));
            }
        }
        func_holder.call<events::guild_update>(packet_handling, true, g);
    } else if (event_name == "GUILD_DELETE") {
        snowflake to_remove = to_sf(data["id"]);
        guilds.erase(std::remove_if(guilds.begin(), guilds.end(), [&to_remove](std::unique_ptr<discord::Guild> const &g) {
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
        process_message_cache<0>(&message, found);
        if (message.author->id != this->id) {
            fire_commands(message);
        }
        func_holder.call<events::message_create>(packet_handling, ready, message);
    } else if (event_name == "MESSAGE_UPDATE") {
        auto message = Message::from_sent_message(data);
        process_message_cache<1>(&message, found);
        func_holder.call<events::message_update>(packet_handling, ready, message);
    } else if (event_name == "MESSAGE_DELETE") {
        // TODO RAW_MESSAGE_DELETE EVENT
        auto message = Message::from_sent_message(data);
        message = process_message_cache<2>(&message, found);
        func_holder.call<events::message_delete>(packet_handling, found, message);
    } else if (event_name == "MESSAGE_DELETE_BULK") {
        for (auto const &each : data) {
            auto message = Message::from_sent_message(each);
            message = process_message_cache<2>(&message, found);
            func_holder.call<events::message_delete>(packet_handling, found, message);
        }
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
    nlohmann::json obj = { { "op", 2 },
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
        while (true) {
            if (!packet_handling.size()){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            packet_handling.erase(packet_handling.begin());
    } } };
}

std::string discord::Bot::get_gateway_url() {
    auto r = send_request<request_method::Get>(nlohmann::json({}), get_basic_header(), format("%/gateway/bot", get_api()));
    if (!r.contains("url")) {
        throw discord::ImproperToken();
    }
    return r["url"];
}

void discord::Bot::register_command(std::string const &command_name, std::function<void(discord::Context const &)> function) {
    command_map[boost::to_lower_copy(command_name)] = function;
}

void discord::Bot::fire_commands(discord::Message &m) const {
    if (!boost::starts_with(m.content, prefix)) {
        return;
    }
    std::vector<std::string> argument_vec;
    boost::split(argument_vec, m.content, boost::is_any_of(" "));
    if (!argument_vec.size()) {
        return;
    }
    auto command_name = argument_vec[0].erase(0, prefix.size());
    if (command_map.find(command_name) == command_map.end()) {
        return;
    }
    /*
        const discord::Bot* bot;
        const discord::Message& message;
        const std::vector<std::string>& arguments;
        const std::function<void(Context const& c)> command;
        const std::string command_name;

    */
    argument_vec.erase(argument_vec.begin());
    auto f = command_map.at(command_name);
    f({ this, m, argument_vec, f, command_name });
}

void discord::Bot::initialize_variables(const std::string raw) {
    nlohmann::json j = nlohmann::json::parse(raw);
    auto user = j["user"];
    discriminator = user["discriminator"];
    id = to_sf(user["id"]);
    verified = user["verified"];
    mfa_enabled = user["mfa_enabled"];
    bot = user["bot"];
    username = user["username"];
    avatar = get_value(j, "avatar", "");
    email = get_value(j, "email", "");
}

cpr::Header discord::Bot::get_basic_header() {
    return cpr::Header{
        { "Authorization", format("Bot %", token) }
    };
}

void discord::Bot::handle_heartbeat() {
    while (true) {
        nlohmann::json data = { { "op", 1 }, { "d", nullptr } };
        if (last_sequence_data != -1) {
            data["d"] = last_sequence_data;
        }
        con->send(data.dump());
        heartbeat_acked = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(hello_packet["d"]["heartbeat_interval"].get<int>()));
        if (!heartbeat_acked) {
            data = nlohmann::json({ { "token", token },
                                    { "session_id", session_id },
                                    { "seq", last_sequence_data } });
            con->send(data.dump());
        }
    }
}

discord::Guild discord::Bot::create_guild(std::string const &name, std::string const &region, int const &verification_level, int const &default_message_notifications, int const &explicit_content_filter) {
    const nlohmann::json data = nlohmann::json({ { "name", name },
                                                 { "region", region },
                                                 { "icon", "" },
                                                 { "verification_level", verification_level },
                                                 { "default_message_notifications", default_message_notifications },
                                                 { "explicit_content_filter", explicit_content_filter },
                                                 { "roles", {} },
                                                 { "channels", {} } });
    return discord::Guild{ send_request<request_method::Post>(data, get_default_headers(), get_create_guild_url()) };
}

void discord::Bot::channel_create_event(nlohmann::json j) {
    const auto data = j["d"];
    auto channel = Channel{ data, to_sf(get_value(data, "guild_id", "0")) };
    if (channel.guild) {
        for (auto &guild : this->guilds) {
            if (guild->id != channel.guild->id) {
                continue;
            }
            guild->channels.push_back(channel);
            break;
        }
    }
    channels.emplace_back(std::make_unique<discord::Channel>(channel));
    func_holder.call<events::channel_create>(packet_handling, true, channel);
}

void discord::Bot::channel_update_event(nlohmann::json j) {
    const nlohmann::json data = j["d"];
    auto channel = Channel{ data, to_sf(get_value(data, "guild_id", "0")) };
    auto g = discord::utils::get(this->guilds, [&channel](auto const &g) {
        return channel.guild->id == g->id;
    });

    for (auto &each : g->channels) {
        if (each.id == channel.id) {
            each = channel;
            return;
        }
    }

    func_holder.call<events::channel_create>(packet_handling, true, channel);
}

void discord::Bot::channel_delete_event(nlohmann::json j) {
    const nlohmann::json data = j["d"];
    snowflake chan_id = to_sf(data["id"]);
    discord::Channel event_chan;
    for (auto &guild : this->guilds) {
        for (std::size_t i = 0; i < guild->channels.size(); i++) {
            if (guild->channels[i].id != chan_id) {
                continue;
            }
            event_chan = guild->channels[i];
            guild->channels.erase(guild->channels.begin() + i);
        }
    }

    for (std::size_t i = 0; i < channels.size(); i++) {
        if (channels[i]->id != chan_id) {
            continue;
        }
        event_chan = *(channels[i]);
        channels.erase(channels.begin() + i);
        break;
    }
    func_holder.call<events::channel_delete>(packet_handling, true, event_chan);
}

void discord::Bot::guild_create_event(nlohmann::json j) {
    const nlohmann::json data = j["d"];
    snowflake guild_id = to_sf(get_value(data, "id", "0"));
    for (auto const &member : data["members"]) {
        auto mem_id = to_sf(member["user"]["id"]);
        for (auto const &each : this->users) {
            if (*each == mem_id) {
                break;
            }
        }
        users.emplace_back(std::make_unique<discord::User>(member["user"]));
    }

    auto guild = discord::Guild(j["d"]);
    guilds.push_back(std::make_unique<discord::Guild>(j["d"]));

    for (auto const &channel : data["channels"]) {
        channels.emplace_back(std::make_unique<discord::Channel>(channel, guild_id));
    }

    if (!ready) {
        for (auto const &unavail_guild : ready_packet["guilds"]) {
            snowflake g_id = to_sf(unavail_guild["id"]);
            if (std::find_if(guilds.begin(), guilds.end(), [&g_id](std::unique_ptr<discord::Guild> &g) { return g_id == g->id; }) == guilds.end()) {
                return;
            }
        }
        ready = true;
        func_holder.call<events::ready>(packet_handling, true);
    }
    func_holder.call<events::guild_create>(packet_handling, ready, guild);
}

template <std::size_t event_type>
discord::Message discord::Bot::process_message_cache(discord::Message *m, bool &found) {
    static_assert(event_type >= 0 && event_type < 3);
    auto return_m = *m;
    if (event_type == 0) {  // create
        if (messages.size() >= message_cache_count) {
            messages.erase(messages.begin());
        }
        messages.push_back(*m);
    } else if (event_type == 1) {  // update
        for (std::size_t i = 0; i < messages.size(); i++) {
            if (messages[i].id != m->id) {
                continue;
            }
            messages[i] = *m;
        }
    } else if (event_type == 2) {  // delete
        for (std::size_t i = 0; i < messages.size(); i++) {
            if (messages[i].id != m->id) {
                continue;
            }
            return_m = messages[i];
            found = true;
            messages.erase(messages.begin() + i);
        }
    }
    return return_m;
}


void discord::Bot::update_presence(Activity const &act) {
    con->send(nlohmann::json({ { "op", 3 }, { "d", act.to_json() } }).dump());
}

std::string discord::Bot::get_create_guild_url() {
    return format("%/guilds", get_api());
}