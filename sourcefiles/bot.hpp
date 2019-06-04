#pragma once
#include "assets.hpp"
#include "context.hpp"
#include "cpr/cpr.h"
#include "discord.hpp"
#include "events.hpp"
#include "exceptions.hpp"
#include "gatewayhandler.hpp"
#include "guild.hpp"
#include "integration.hpp"
#include "message.hpp"
#include "nlohmann/json.hpp"

discord::Bot::Bot(const std::string &token, const std::string prefix, std::size_t message_cache_count)
    : ready{ false }, token{ token }, prefix{ prefix }, message_cache_count{ message_cache_count } {
    discord::detail::bot_instance = this;
    internal_event_map["HELLO"] = std::bind(&discord::Bot::hello_event, this, std::placeholders::_1);
    internal_event_map["READY"] = std::bind(&discord::Bot::ready_event, this, std::placeholders::_1);
    internal_event_map["RESUMED"] = std::bind(&discord::Bot::resumed_event, this, std::placeholders::_1);
    internal_event_map["INVALID_SESSION"] = std::bind(&discord::Bot::invalid_session_event, this, std::placeholders::_1);
    internal_event_map["CHANNEL_CREATE"] = std::bind(&discord::Bot::channel_create_event, this, std::placeholders::_1);
    internal_event_map["CHANNEL_UPDATE"] = std::bind(&discord::Bot::channel_update_event, this, std::placeholders::_1);
    internal_event_map["CHANNEL_DELETE"] = std::bind(&discord::Bot::channel_delete_event, this, std::placeholders::_1);
    internal_event_map["CHANNEL_PINS_UPDATE"] = std::bind(&discord::Bot::channel_pins_update_event, this, std::placeholders::_1);
    internal_event_map["GUILD_CREATE"] = std::bind(&discord::Bot::guild_create_event, this, std::placeholders::_1);
    internal_event_map["GUILD_UPDATE"] = std::bind(&discord::Bot::guild_update_event, this, std::placeholders::_1);
    internal_event_map["GUILD_DELETE"] = std::bind(&discord::Bot::guild_delete_event, this, std::placeholders::_1);
    internal_event_map["GUILD_BAN_ADD"] = std::bind(&discord::Bot::guild_ban_add_event, this, std::placeholders::_1);
    internal_event_map["GUILD_BAN_REMOVE"] = std::bind(&discord::Bot::guild_ban_remove_event, this, std::placeholders::_1);
    internal_event_map["GUILD_EMOJIS_UPDATE"] = std::bind(&discord::Bot::guild_emojis_update_event, this, std::placeholders::_1);
    internal_event_map["GUILD_INTEGRATIONS_UPDATE"] = std::bind(&discord::Bot::guild_integrations_update_event, this, std::placeholders::_1);
    internal_event_map["GUILD_MEMBER_ADD"] = std::bind(&discord::Bot::guild_member_add_event, this, std::placeholders::_1);
    internal_event_map["GUILD_MEMBER_REMOVE"] = std::bind(&discord::Bot::guild_member_remove_event, this, std::placeholders::_1);
    internal_event_map["GUILD_MEMBER_UPDATE"] = std::bind(&discord::Bot::guild_member_update_event, this, std::placeholders::_1);
    internal_event_map["GUILD_MEMBERS_CHUNK"] = std::bind(&discord::Bot::guild_members_chunk_event, this, std::placeholders::_1);
    internal_event_map["GUILD_ROLE_CREATE"] = std::bind(&discord::Bot::guild_role_create_event, this, std::placeholders::_1);
    internal_event_map["GUILD_ROLE_UPDATE"] = std::bind(&discord::Bot::guild_role_update_event, this, std::placeholders::_1);
    internal_event_map["GUILD_ROLE_DELETE"] = std::bind(&discord::Bot::guild_role_delete_event, this, std::placeholders::_1);
    internal_event_map["MESSAGE_CREATE"] = std::bind(&discord::Bot::message_create_event, this, std::placeholders::_1);
    internal_event_map["MESSAGE_UPDATE"] = std::bind(&discord::Bot::message_update_event, this, std::placeholders::_1);
    internal_event_map["MESSAGE_DELETE"] = std::bind(&discord::Bot::message_delete_event, this, std::placeholders::_1);
    internal_event_map["MESSAGE_DELETE_BULK"] = std::bind(&discord::Bot::message_delete_bulk_event, this, std::placeholders::_1);
    internal_event_map["MESSAGE_REACTION_ADD"] = std::bind(&discord::Bot::message_reaction_add_event, this, std::placeholders::_1);
    internal_event_map["MESSAGE_REACTION_REMOVE"] = std::bind(&discord::Bot::message_reaction_remove_event, this, std::placeholders::_1);
    internal_event_map["MESSAGE_REACTION_REMOVE_ALL"] = std::bind(&discord::Bot::message_reaction_remove_all_event, this, std::placeholders::_1);
    internal_event_map["PRESENCE_UPDATE"] = std::bind(&discord::Bot::presence_update_event, this, std::placeholders::_1);
    internal_event_map["TYPING_START"] = std::bind(&discord::Bot::typing_start_event, this, std::placeholders::_1);
    internal_event_map["USER_UPDATE"] = std::bind(&discord::Bot::user_update_event, this, std::placeholders::_1);
    internal_event_map["VOICE_STATE_UPDATE"] = std::bind(&discord::Bot::voice_state_update_event, this, std::placeholders::_1);
    internal_event_map["VOICE_SERVER_UPDATE"] = std::bind(&discord::Bot::voice_server_update_event, this, std::placeholders::_1);
    internal_event_map["WEBHOOKS_UPDATE"] = std::bind(&discord::Bot::webhooks_update_event, this, std::placeholders::_1);
}

discord::Message discord::Bot::send_message(snowflake channel_id, std::string message_content, bool tts) {
    nlohmann::json j = nlohmann::json({ { "content", message_content }, { "tts", tts } });
    auto response = send_request<request_method::Post>(j, get_default_headers(), get_channel_link(channel_id));
    return discord::Message{ response };
}

discord::Message discord::Bot::send_message(snowflake channel_id, nlohmann::json message_content, bool tts) {
    message_content["tts"] = tts;
    auto response = send_request<request_method::Post>(message_content, get_default_headers(), get_channel_link(channel_id));
    return discord::Message{ response };
}

void discord::Bot::on_incoming_packet(const websocketpp::connection_hdl &, const client::message_ptr &msg) {
    nlohmann::json j = nlohmann::json::parse(msg->get_payload());
    switch (j["op"].get<int>()) {
        case (9):
            if (disconnected) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                con->send(get_identify_packet());
            }
            break;
        case (10):
            hello_packet = j;
            con->send(get_identify_packet());
            break;
        case (11):
            heartbeat_acked = true;
            break;
        default:
            handle_event(j, get_value(j, "t", ""));
            break;
    }
    packet_counter++;
}

void discord::Bot::handle_gateway() {
    std::string hostname = "discord.gg";
    std::string uri = get_gateway_url();
    try {
#ifdef __DPP_DEBUG
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        c.set_error_channels(websocketpp::log::elevel::all);
#else
        c.set_access_channels(websocketpp::log::alevel::none);
#endif
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
#ifdef __DPP_DEBUG
        std::cout << e.what() << std::endl;
#endif
    }
}

int discord::Bot::run() {
    gateway_auth();
    while (true) {
        if (!futures.size()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        futures.erase(futures.begin());
    }
    return 0;
}

std::string discord::Bot::get_identify_packet() {
    nlohmann::json obj = { { "op", 2 },
                           { "d",
                             { { "token", token },
                               { "properties",
                                 { { "$os", get_os_name() },
                                   { "$browser", "DiscordPP" },
                                   { "$device", "DiscordPP" } } },
                               { "compress", false },
                               { "large_threshold", 250 } } } };
    return obj.dump();
}

void discord::Bot::gateway_auth() {
    gateway_thread = std::thread{ &Bot::handle_gateway, this };
}

std::string discord::Bot::get_gateway_url() const {
    auto r = send_request<request_method::Get>(nlohmann::json({}), get_basic_header(), endpoint("/gateway/bot"));
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
    if (j.contains("avatar")) {
        if (j["avatar"].is_null()) {
            avatar = { "", default_user_avatar, false, to_sf(discriminator) };
        } else {
            std::string av_hash = j["avatar"];
            avatar = { j["avatar"], user_avatar, av_hash[0] == 'a' && av_hash[1] == '_', id };
        }
    }
    email = get_value(j, "email", "");
}

cpr::Header discord::Bot::get_basic_header() const {
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
            disconnected = true;
            data = nlohmann::json({ { "token", token },
                                    { "session_id", session_id },
                                    { "seq", last_sequence_data } });
            con->send(data.dump());
        }
    }
}

discord::Guild discord::Bot::create_guild(std::string const &name, std::string const &region, int const &verification_level, int const &default_message_notifications, int const &explicit_content_filter) {
    return discord::Guild{
        send_request<request_method::Post>(nlohmann::json({ { "name", name },
                                                            { "region", region },
                                                            { "icon", "" },
                                                            { "verification_level", verification_level },
                                                            { "default_message_notifications", default_message_notifications },
                                                            { "explicit_content_filter", explicit_content_filter },
                                                            { "roles", {} },
                                                            { "channels", {} } }),
                                           get_default_headers(),
                                           endpoint("/guilds"))
    };
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

std::vector<discord::VoiceRegion> discord::Bot::get_voice_regions() const {
    std::vector<VoiceRegion> return_vec = {};
    auto response = send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), endpoint("%/voice/regions"));
    for (auto const &each : response) {
        return_vec.push_back({ get_value(each, "id", ""),
                               get_value(each, "name", ""),
                               get_value(each, "vip", false),
                               get_value(each, "optimal", false),
                               get_value(each, "deprecated", false),
                               get_value(each, "custom", false) });
    }
    return return_vec;
}

void discord::Bot::update_presence(Activity const &act) {
    con->send(nlohmann::json({ { "op", 3 }, { "d", act.to_json() } }).dump());
}

void discord::Bot::handle_event(nlohmann::json const j, std::string event_name) {
    const nlohmann::json data = j["d"];
    last_sequence_data = j["s"].is_number() && j.contains("s") ? j["s"].get<int>() : -1;
    std::printf("Incoming event: %s\n", event_name.c_str());
    if (internal_event_map.find(event_name) != internal_event_map.end()) {
        internal_event_map[event_name](data);
    } else {
#ifdef __DPP_DEBUG
        std::cout << "Unknown event: " << event_name << std::endl;
#endif
    }
}

void discord::Bot::hello_event(nlohmann::json) {
    func_holder.call<events::hello>(futures, true);
}

void discord::Bot::ready_event(nlohmann::json data) {
    session_id = data["session_id"];
    heartbeat_thread = std::thread{ &Bot::handle_heartbeat, this };
    initialize_variables(data.dump());
    ready_packet = data;
}

void discord::Bot::resumed_event(nlohmann::json) {
    func_holder.call<events::resumed>(futures, true);
}

void discord::Bot::invalid_session_event(nlohmann::json) {
    func_holder.call<events::invalid_session>(futures, true);
}

void discord::Bot::channel_create_event(nlohmann::json j) {
    const auto data = j["d"];

    auto channel = std::make_shared<discord::Channel>(data, to_sf(get_value(data, "guild_id", "0")));
    if (channel->guild) {
        auto guild = discord::utils::get(guilds, [channel](auto &guild) {
            return channel->guild->id == guild->id;
        });
        if (guild) {
            guild->channels.push_back(channel);
        }
    }
    channels.push_back(channel);
    func_holder.call<events::channel_create>(futures, true, channel);
}

void discord::Bot::channel_update_event(nlohmann::json j) {
    const nlohmann::json data = j["d"];
    auto c_id = to_sf(data["id"]);
    auto channel = discord::utils::get(channels, [=](auto &chan) {
        return chan->id == c_id;
    });

    channel->update(data);

    func_holder.call<events::channel_create>(futures, true, channel);
}

void discord::Bot::channel_delete_event(nlohmann::json j) {
    const nlohmann::json data = j["d"];
    snowflake chan_id = to_sf(get_value(data, "id", "0"));
    std::shared_ptr<discord::Channel> event_chan;
    if (!chan_id) {
        return func_holder.call<events::channel_delete>(futures, true, event_chan);
    }

    for (auto &guild : guilds) {
        for (size_t i = 0; i < guild->channels.size(); i++) {
            if (guild->channels[i]->id == chan_id) {
                event_chan = guild->channels[i];
                guild->channels.erase(guild->channels.begin() + i);
            }
        }
    }

    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i]->id == chan_id) {
            if (!event_chan) {
                event_chan = channels[i];
            }
            channels.erase(channels.begin() + i);
        }
    }

    func_holder.call<events::channel_delete>(futures, true, event_chan);
}

void discord::Bot::channel_pins_update_event(nlohmann::json data) {
    auto c_id = to_sf(data["channel_id"]);
    auto channel = discord::utils::get(channels, [=](auto const &chn) { return chn->id == c_id; });
    func_holder.call<events::channel_pins_update>(futures, true, channel,
                                                  time_from_discord_string(get_value(data, "last_pin_timestamp", "")));
}

void discord::Bot::guild_create_event(nlohmann::json data) {
    snowflake guild_id = to_sf(data["id"]);
    auto guild = discord::utils::get(guilds, [guild_id](auto &gld) { return gld->id == guild_id });
    if (guild) {
        guild->update(data);
        return func_holder.call<events::guild_create>(futures, ready, guild);
    }

    auto guild = std::make_shared<discord::Guild>(data);
    guilds.push_back(guild);

    for (auto const &member : data["members"]) {
        auto usr_id = to_sf(data["user"]["id"]);
        auto usr = std::make_shared<discord::User>(member["user"]);
        if (std::find_if(users.begin(), users.end(),
                         [usr_id](auto const &usr_ptr) { return usr_ptr->id == usr_id; }) != users.end()) {
            users.push_back(usr);
        }
    }

    for (auto const &channel : data["channels"]) {
        auto chan = std::make_shared<discord::Channel>(channel, guild_id);
        channels.push_back(chan);
        guild->channels.push_back(chan);
    }

    if (!ready) {
        for (auto const &unavail_guild : ready_packet["guilds"]) {
            snowflake g_id = to_sf(unavail_guild["id"]);
            if (std::find_if(guilds.begin(), guilds.end(), [g_id](auto const &g) { return g_id == g->id; }) == guilds.end()) {
                return;
            }
        }
        ready = true;
        func_holder.call<events::ready>(futures, ready);
    }
    func_holder.call<events::guild_create>(futures, ready, guild);
}

void discord::Bot::guild_update_event(nlohmann::json data) {
    snowflake updated_guild = to_sf(data["id"]);
    auto g = discord::utils::get(guilds, [updated_guild](auto &gld) { return gld->id == new_guild; });
    g->update(data);
    func_holder.call<events::guild_update>(futures, ready, g);
}

void discord::Bot::guild_delete_event(nlohmann::json data) {
    snowflake to_remove = to_sf(data["id"]);
    std::shared_ptr<discord::Guild> g_ptr;
    for (size_t i = 0; i < guilds.size(); i++) {
        if (guilds[i]->id == to_remove) {
            g_ptr = guilds[i];
            guilds.erase(guilds.begin() + i);
        }
    }
    func_holder.call<events::guild_delete>(futures, ready, g_ptr);
}

void discord::Bot::guild_ban_add_event(nlohmann::json data) {
    discord::Guild banned_guild{ to_sf(data["guild_id"]) };
    discord::User user{ data["user"] };
    func_holder.call<events::guild_ban_add>(futures, ready, banned_guild, user);
}

void discord::Bot::guild_ban_remove_event(nlohmann::json data) {
    discord::Guild banned_guild{ to_sf(data["guild_id"]) };
    discord::User user{ data["user"] };
    func_holder.call<events::guild_ban_remove>(futures, ready, banned_guild, user);
}

void discord::Bot::guild_emojis_update_event(nlohmann::json data) {
    discord::Guild emote_guild{ to_sf(get_value(data, "guild", "0")) };
    auto emote_vec = from_json_array<discord::Emoji>(data["emojis"]);
    func_holder.call<events::guild_emojis_update>(futures, ready, emote_guild, emote_vec);
}

void discord::Bot::guild_integrations_update_event(nlohmann::json data) {
    func_holder.call<events::guild_integrations_update>(
        futures, ready, discord::Guild{ to_sf(data["guild_id"]) });
}

void discord::Bot::guild_member_add_event(nlohmann::json data) {
    snowflake guild_id = to_sf(data["guild_id"]);
    auto guild = discord::utils::get(this->guilds, [guild_id](auto &g) {
        return g->id == guild_id;
    });
    discord::User user{ data["user"] };
    std::shared_ptr<discord::Member> mem = std::make_shared<discord::Member>(data, user, guild.get());
    guild->members.emplace_back(mem.get());
    func_holder.call<events::guild_member_add>(futures, ready, *mem);
}

void discord::Bot::guild_member_remove_event(nlohmann::json data) {
    discord::User user{ data["user"] };
    snowflake guild_id = to_sf(data["guild_id"]);
    auto guild = discord::utils::get(this->guilds, [guild_id](auto &g) {
        return g->id == guild_id;
    });
    guild->members.erase(std::remove_if(guild->members.begin(), guild->members.end(), [&user](auto const &i) {
                             return i->id == user.id;
                         }),
                         guild->members.end());
    func_holder.call<events::guild_member_remove>(futures, ready, user);
}

void discord::Bot::guild_member_update_event(nlohmann::json data) {
    discord::User user{ data["user"] };
    snowflake guild_id = to_sf(data["guild_id"]);
    auto guild = discord::utils::get(this->guilds, [guild_id](auto &g) {
        return g->id == guild_id;
    });
    auto member = discord::utils::get(guild->members, [user](auto &usr) {
        return usr->id == user.id;
    });
    member->nick = get_value(data, "nick", member->nick);  // member is 0x0
    member->roles.clear();
    for (auto const &each_id : data["roles"]) {
        member->roles.emplace_back(to_sf(each_id));
    }
    func_holder.call<events::guild_member_update>(futures, ready, *member);
}

// TODO: implement
void discord::Bot::guild_members_chunk_event(nlohmann::json) {
}
void discord::Bot::guild_role_create_event(nlohmann::json) {
}
void discord::Bot::guild_role_update_event(nlohmann::json) {
}
void discord::Bot::guild_role_delete_event(nlohmann::json) {
}

void discord::Bot::message_create_event(nlohmann::json data) {
    bool found = false;
    auto message = Message{ data };
    process_message_cache<0>(&message, found);
    if (message.author && message.author->id != this->id) {
        fire_commands(message);
    }
    func_holder.call<events::message_create>(futures, ready, message);
}

void discord::Bot::message_update_event(nlohmann::json data) {
    bool found = false;
    auto message = Message{ data };
    process_message_cache<1>(&message, found);
    func_holder.call<events::message_update>(futures, ready, message);
}

void discord::Bot::message_delete_event(nlohmann::json data) {
    // TODO RAW_MESSAGE_DELETE EVENT
    bool found = false;
    auto message = Message{ data };
    message = process_message_cache<2>(&message, found);
    func_holder.call<events::message_delete>(futures, found, message);
}

void discord::Bot::message_delete_bulk_event(nlohmann::json data) {
    for (auto const &each : data) {
        bool found = false;
        auto message = Message{ each };
        message = process_message_cache<2>(&message, found);
        func_holder.call<events::message_delete>(futures, found, message);
    }
}

// TODO: implement
void discord::Bot::message_reaction_add_event(nlohmann::json) {
}
void discord::Bot::message_reaction_remove_event(nlohmann::json) {
}
void discord::Bot::message_reaction_remove_all_event(nlohmann::json) {
}
void discord::Bot::presence_update_event(nlohmann::json) {
}
void discord::Bot::typing_start_event(nlohmann::json) {
}
void discord::Bot::user_update_event(nlohmann::json) {
}
void discord::Bot::voice_state_update_event(nlohmann::json) {
}
void discord::Bot::voice_server_update_event(nlohmann::json) {
}
void discord::Bot::webhooks_update_event(nlohmann::json) {
}

discord::User discord::Bot::get_current_user() {
    return discord::User{ send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), endpoint("/users/@me")) };
}

discord::User discord::Bot::get_user(snowflake id) {
    return discord::User{ send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), endpoint("/users/%", id)) };
}

discord::User discord::Bot::edit(std::string const &username) {
    discord::User user{ send_request<request_method::Patch>(
        nlohmann::json({ { "username", username } }),
        get_default_headers(),
        endpoint("/users/@me")) };
    this->username = user.name;
    this->avatar = user.avatar;
    return user;
}

std::vector<discord::Guild> discord::Bot::get_user_guilds(int limit, snowflake before, snowflake after) {
    std::vector<discord::Guild> g_vec{};
    nlohmann::json data({ { "limit", limit ? limit : 100 } });

    if (before) {
        data["before"] = before;
    }

    if (after) {
        data["after"] = after;
    }

    auto d = send_request<request_method::Get>(data, get_default_headers(), endpoint("/users/@me/guilds"));
    for (auto const &each : d) {
        snowflake guild_id = to_sf(each["id"]);
        g_vec.push_back(*discord::utils::get(this->guilds, [guild_id](auto const &guild) {
            return guild->id == guild_id;
        }));
    }
    return g_vec;
}

discord::Channel discord::Bot::create_group_dm(std::vector<std::string> const &access_tokens, nlohmann::json const &nicks) {
    nlohmann::json data({ { "access_tokens", nlohmann::json::array() }, { "nicks", nicks } });
    for (auto const &each : access_tokens) {
        data["access_tokens"].push_back(each);
    }
    return discord::Channel{
        send_request<request_method::Post>(data, get_default_headers(), endpoint("/users/@me/channels"))
    };
}

std::vector<discord::Connection> discord::Bot::get_connections() {
    std::vector<discord::Connection> conn_vec;
    auto response = send_request<request_method::Get>(
        nlohmann::json({}),
        get_default_headers(),
        endpoint("/users/@me/connections"));
    for (auto const &each : response) {
        conn_vec.push_back({ to_sf(each["id"]),
                             each["name"],
                             each["type"],
                             each["revoked"],
                             from_json_array<discord::Integration>(each["integrations"]),
                             each["verified"],
                             each["friend_sync"],
                             each["show_activity"],
                             each["visibility"] });
    }
    return conn_vec;
}

discord::Guild discord::Bot::get_guild(snowflake g_id) {
    return discord::Guild{
        send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), endpoint("/guilds/%", g_id))
    };
}

discord::Channel discord::Bot::get_channel(snowflake chan_id) {
    return discord::Channel{
        send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), endpoint("/channels/%", chan_id))
    };
}