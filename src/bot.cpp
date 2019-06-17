#include "bot.hpp"
#include "context.hpp"
#include "emoji.hpp"
#include "events.hpp"
#include "exceptions.hpp"
#include "gatewayhandler.hpp"
#include "guild.hpp"
#include "integration.hpp"
#include "member.hpp"
#include "utils.hpp"

#include <boost/algorithm/string.hpp>

namespace discord {

    Bot::Bot(const std::string &token, const std::string prefix, std::size_t message_cache_count)
        : ready{ false }, token{ token }, prefix{ prefix }, message_cache_count{ message_cache_count } {
        /**
         * @brief Constructs a discord::Bot object
         * 
         * ```cpp
         *      discord::Bot bot{TOKEN, ".", 5000};
         * ```
         * 
         * @param[in] token The discord bot token which should be used for running your bot.
         * @param[in] prefix The bot's prefix that should be used for command handling.
         * @param[in] message_cache_count Amount of messages that should be cached by the bot. Useful for message_delete and message_update events.
         * 
         * No more than one discord::Bot should be constructed and ran at a time to prevent using tokens that don't correspond with their bot instances.
         * @return discord::Bot, this is a constructor
         */
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

    void Bot::on_incoming_packet(const websocketpp::connection_hdl &, const client::message_ptr &msg) {
        /**
         * @brief Incoming packet handler
         * 
         * @param[in] msg Incoming message from the discord gateway.
         * 
         * @return void
         */
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

    void Bot::handle_gateway() {
        /**
         * @brief Gateway handler
         * 
         * Listens to packets incoming from the discord gateway.
         * 
         * @return void
         */
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

    int Bot::run() {
        /**
         * @brief Runs your discord bot
         * 
         * Runs your discord bot, handles your event loop and launches your gateway handling thread.
         * 
         * Keep in mind that this call is blocking indefinitely, meaning that you can only call this once.
         * This function should be ran in your main() function, and should probably be `return`'ed from main.
         * 
         * @return int, always 0 for now.
         */
        gateway_thread = std::thread{ &Bot::handle_gateway, this };
        while (true) {
            for (size_t i = 0; i < futures.size(); i++) {
                if (!futures[i].valid() || !(futures[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready)) {
                    continue;
                }
                futures.erase(futures.begin() + i);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return 0;
    }

    std::string Bot::get_identify_packet() {
        /**
         * @brief Returns identifying packet that will be sent to the discord gateway when the "hello" packet is receieved
         * 
         * @return std::string Returns the UTF-8 string that will be sent.
         */
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

    std::string Bot::get_gateway_url() const {
        /**
         * @brief Blocking call to the discord api for receiving the websocket url to connect to.
         * 
         * Blocking call to the discord API.
         * Checks whether you can start a session, and if so, will return the websocket url.
         * 
         * @throws discord::ImproperToken Throws if response does not contains key "url".
         * @throws std::runtime_error Throws if you cannot start anymore sessions due to limit being reached.
         * 
         * @return std::string Returns the URL to the websocket that Bot::run() will connect to.
         */
        auto r = send_request(methods::GET, endpoint("/gateway/bot"), 0, global).get().unwrap();
        if (!r.contains("url")) {
            throw discord::ImproperToken();
        }
        if (r["session_start_limit"]["remaining"].get<int>() == 0) {
            throw std::runtime_error{ "Maximum start limit reached" };
        }
        // if (shard_count == 0){
        //     auto shard_count = r["shards"].get<int>();
        // }
        return r["url"];
    }

    void Bot::register_command(std::string const &command_name, std::function<void(discord::Context)> function) {
        /**
         * @brief Registers a command.
         * 
         * Registers a command that will call \ref function when \ref prefix + \ref command_name has been received in any channel that the bot has access to.
         * 
         * Example:
         * ```cpp
         *     discord::Bot bot{".", TOKEN};
         *     bot.register_command("hello", [](discord::Context ctx){
         *          ctx.channel->send("Hello!").wait();
         *     });
         * 
         *     bot.run();
         * ```
         * 
         * This will trigger when someone types ".hello" in any chat.
         * 
         * This command is case sensitive, so "hello" will not be triggered when you type ".HELLO"
         * 
         * @param[in] command_name String to which \ref function should be called.
         * @param[in] function Function that should be triggered when command with \ref command_name is fired.
         * 
         * @return void
         */
        command_map[boost::to_lower_copy(command_name)] = function;
    }

    void Bot::fire_commands(discord::Message &m) {
        /**
         * @brief Internal function used for firing commands registered using register_command
         * 
         * Handles incoming messages and fires their corresponding commands, if any.
         * If no commands are found the function gracefully returns.
         * 
         * @return void
         */
        if (m.content.find(prefix) == std::string::npos) {
            return;
        }

        std::vector<std::string> argument_vec{};
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

        futures.push_back(std::async(std::launch::async, f, discord::Context{ this, m, argument_vec, f, command_name }));
    }

    void Bot::initialize_variables(const std::string raw) {
        /**
         * @brief Initializes variables like \ref id and \ref username of the current user (Your bot).
         * 
         * @throws nlohmann::detail::type_error if types do not match
         * @throws nlohmann::detail::parse_error if input is invalid
         * @throws Assert if key is not found
         * 
         * @return void
         */
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

    void Bot::handle_heartbeat() {
        /**
         * @brief Handles heartbeat that's required to be sent every \ref hello_packet["d"]["heartbeat_interval"] to discord's gateway
         * 
         * @throws nlohmann::detail::type_error if "heartbeat_interval" is not int.
         * @throws Assert if hello_packet does not contain "d" or hello_packet["d"] does not contain "heartbeat_interval"
         * 
         * Indefinitely blocking.
         * 
         * @return void
         */
        while (true) {
            nlohmann::json data = { { "op", 1 }, { "d", nullptr } };
            if (last_sequence_data != -1) {
                data["d"] = last_sequence_data;
            }

            con->send(data.dump());
            heartbeat_acked = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(hello_packet["d"]["heartbeat_interval"].get<int>() - 5));
            if (!heartbeat_acked) {
                disconnected = true;
                data = nlohmann::json({ { "token", token },
                                        { "session_id", session_id },
                                        { "seq", last_sequence_data } });
                con->send(data.dump());
            }
        }
    }

    pplx::task<discord::Guild> Bot::create_guild(std::string const &name, std::string const &region, int const &verification_level, int const &default_message_notifications, int const &explicit_content_filter) {
        /**
         * @brief Creates a new Guild.
         * 
         * This method should only be used if your bot is in less than 10 guilds, otherwise this will result in Undefined Behavior
         * 
         * ```cpp
         *     discord::Guild new_guild = bot.create_guild("My test guild", "us-west", 0, 0, 1).get();
         *     for (auto& channel : new_guild.channels){
         *         if (channel->type == guild_text_channel) {
         *             channel->send("Hello! This is a newly created guild!").wait();
         *         }
         *     }
         * ```
         * 
         * @param[in] name Name of the guild that's going to get created
         * @param[in] region Region that the guild is going to be based in on discord's servers, these regions are obtainable through the \ref get_voice_regions function.
         * @param[in] verification_level Verification level of the guild 0 for None, 1 for Low, 2 for Medium, 3 for High, 4 for Very high.
         * @param[in] default_message_notifications Default message notification level, 0 for all messages, 1 for mentions only
         * @param[in] explicit_content_filter Explicit content filter in the guild, 0 for disabled, 1 for members without roles, 2 for all members
         * 
         * @throws discord::Guild::Guild Anything that the discord::Guild constructor can throw
         * 
         * @return pplx::task<discord::Guild> A task that will eventually yield a discord::Guild object of your newly created guild.
         */

        return send_request(methods::POST, endpoint("/guilds"), 0, global,
                            { { "name", name },
                              { "region", region },
                              { "icon", "" },
                              { "verification_level", verification_level },
                              { "default_message_notifications", default_message_notifications },
                              { "explicit_content_filter", explicit_content_filter },
                              { "roles", {} },
                              { "channels", {} } })
            .then([](pplx::task<Result<nlohmann::json>> const &resp) {
                return discord::Guild{ resp.get().unwrap() };
            });
    }

    pplx::task<std::vector<Bot::VoiceRegion>> Bot::get_voice_regions() const {
        /**
         * @brief Lists all available discord voice regions.
         * 
         * @throws nlohmann::json::parse_error if discord's response is not a valid response.
         * 
         * Returns an std::vector of VoiceRegino objects, which can be used for creating guilds, etc.
         * Keep in mind that it is not guaranteed that all the VoiceRegion objects are properly constructed.
         * The members that are not contained in discord's response will be default initialized if discord's response is valid JSON but does not contain all keys.
         * 
         * ```cpp
         *      for (VoiceRegion const& region : bot.get_voice_regions()) {
         *          if (!region.deprecated) {
         *              bot.create_guild("Test guild", region.name).wait();
         *              break;
         *          }
         *      }
         * ```
         * 
         * @return pplx::task<std::vector<VoiceRegion>> Which eventually yields a vector with VoiceRegion objects.
         */
        return send_request(methods::GET, endpoint("%/voice/regions"), 0, global).then([&](pplx::task<Result<nlohmann::json>> const &resp) {
            std::vector<VoiceRegion> return_vec = {};

            for (auto const &each : resp.get().unwrap()) {
                return_vec.push_back({ get_value(each, "id", ""),
                                       get_value(each, "name", ""),
                                       get_value(each, "vip", false),
                                       get_value(each, "optimal", false),
                                       get_value(each, "deprecated", false),
                                       get_value(each, "custom", false) });
            }
            return return_vec;
        });
    }

    void Bot::update_presence(Activity const &act) {
        /**
         * @brief Updates the bot's Presence, the current "playing" or "streaming", etc.
         * 
         * ```cpp
         *      bot.update_presence(
         *          discord::Activity{
         *              discord::format("to % guilds", bot.guilds.size()),
         *              presence::activity::listening,
         *              presence::status::dnd,
         *              false
         *          }
         *      )
         * ```
         * 
         * @param[in] act A discord::Activity object, constructed through discord::Activity::Activity(std::string const& name, presence::activity const& type, std::string const& status, bool const& afk, std::string const& url);
         * 
         * @return void
         */
        con->send(nlohmann::json({ { "op", 3 }, { "d", act.to_json() } }).dump());
    }

    void Bot::handle_event(nlohmann::json const j, std::string event_name) {
        const nlohmann::json data = j["d"];
        last_sequence_data = j["s"].is_number() && j.contains("s") ? j["s"].get<int>() : -1;
#ifdef __DPP_DEBUG
        std::cout << "Incoming event: " << event_name << std::endl;
#endif
        if (internal_event_map.find(event_name) != internal_event_map.end()) {
            if (!ready) {
                internal_event_map[event_name](data);
            } else {
                futures.push_back(std::async(std::launch::async, internal_event_map[event_name], data));
            }
        } else {
#ifdef __DPP_DEBUG
            std::cout << "Unknown event: " << event_name << std::endl;
#endif
        }
    }

    void Bot::hello_event(nlohmann::json) {
        func_holder.call<events::hello>(futures, true);
    }

    void Bot::ready_event(nlohmann::json data) {
        session_id = data["session_id"];
        heartbeat_thread = std::thread{ &Bot::handle_heartbeat, this };
        initialize_variables(data.dump());
        ready_packet = data;
        ready = static_cast<bool>(ready_packet["guilds"].size());
        func_holder.call<events::ready>(futures, ready);
    }

    void Bot::resumed_event(nlohmann::json) {
        func_holder.call<events::resumed>(futures, true);
    }

    void Bot::invalid_session_event(nlohmann::json) {
        func_holder.call<events::invalid_session>(futures, true);
    }

    void Bot::channel_create_event(nlohmann::json data) {
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
        func_holder.call<events::channel_create>(futures, true, *channel);
    }

    void Bot::channel_update_event(nlohmann::json data) {
        auto c_id = to_sf(data["id"]);
        auto channel = discord::utils::get(channels, [=](auto &chan) {
            return chan->id == c_id;
        });

        channel->update(data);

        func_holder.call<events::channel_create>(futures, true, *channel);
    }

    void Bot::channel_delete_event(nlohmann::json j) {
        const nlohmann::json data = j["d"];
        snowflake chan_id = to_sf(get_value(data, "id", "0"));
        std::shared_ptr<discord::Channel> event_chan;
        if (!chan_id) {
            return func_holder.call<events::channel_delete>(futures, true, discord::Channel{});
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

        func_holder.call<events::channel_delete>(futures, true, *event_chan);
    }

    void Bot::channel_pins_update_event(nlohmann::json data) {
        auto c_id = to_sf(data["channel_id"]);
        auto channel = discord::utils::get(channels, [=](auto const &chn) { return chn->id == c_id; });
        func_holder.call<events::channel_pins_update>(futures, true, *channel,
                                                      time_from_discord_string(get_value(data, "last_pin_timestamp", "")));
    }

    void Bot::guild_create_event(nlohmann::json data) {
        snowflake guild_id = to_sf(data["id"]);
        auto guild = discord::utils::get(guilds, [guild_id](auto &gld) { return gld->id == guild_id; });
        if (guild) {
            guild->update(data);
            return func_holder.call<events::guild_create>(futures, ready, *guild);
        }

        for (auto const &member : data["members"]) {
            auto usr_id = to_sf(member["user"]["id"]);
            auto usr = std::make_shared<discord::User>(member["user"]);
            if (std::find_if(users.begin(), users.end(), [usr_id](auto const &usr_ptr) { return usr_ptr->id == usr_id; }) == users.end()) {
                users.push_back(usr);
            }
        }

        guild = std::make_shared<discord::Guild>(data);
        guilds.push_back(guild);

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
        func_holder.call<events::guild_create>(futures, ready, *guild);
    }

    void Bot::guild_update_event(nlohmann::json data) {
        snowflake updated_guild = to_sf(data["id"]);
        auto g = discord::utils::get(guilds, [updated_guild](auto &gld) { return gld->id == updated_guild; });
        g->update(data);
        func_holder.call<events::guild_update>(futures, ready, *g);
    }

    void Bot::guild_delete_event(nlohmann::json data) {
        snowflake to_remove = to_sf(data["id"]);
        std::shared_ptr<discord::Guild> g_ptr;
        for (size_t i = 0; i < guilds.size(); i++) {
            if (guilds[i]->id == to_remove) {
                g_ptr = guilds[i];
                guilds.erase(guilds.begin() + i);
            }
        }
        func_holder.call<events::guild_delete>(futures, ready, *g_ptr);
    }

    void Bot::guild_ban_add_event(nlohmann::json data) {
        snowflake banned_guild = to_sf(data["guild_id"]);
        auto g = discord::utils::get(guilds, [banned_guild](auto &gld) {
            return gld->id == banned_guild;
        });

        snowflake usr_id = to_sf(data["user"]["id"]);
        for (size_t i = 0; i < g->members.size(); i++) {
            if (g->members[i]->id == usr_id) {
                g->members.erase(g->members.begin() + i);
            }
        }

        std::shared_ptr<discord::User> usr_ptr = discord::utils::get(users, [usr_id](auto const &usr) { return usr->id == usr_id; });
        if (!usr_ptr) {
            usr_ptr = std::make_shared<discord::User>(get_user(usr_id).get());
            users.push_back(usr_ptr);
        }
        func_holder.call<events::guild_ban_add>(futures, ready, *g, *usr_ptr);
    }

    void Bot::guild_ban_remove_event(nlohmann::json data) {
        snowflake banned_guild = to_sf(data["guild_id"]);
        auto g = discord::utils::get(guilds, [banned_guild](auto &gld) {
            return gld->id == banned_guild;
        });

        snowflake usr_id = to_sf(data["user"]["id"]);

        std::shared_ptr<discord::User> usr_ptr = discord::utils::get(users, [usr_id](auto const &usr) { return usr->id == usr_id; });
        if (!usr_ptr) {
            usr_ptr = std::make_shared<discord::User>(data["user"]);
            users.push_back(usr_ptr);
        }
        func_holder.call<events::guild_ban_remove>(futures, ready, *g, *usr_ptr);
    }

    void Bot::guild_emojis_update_event(nlohmann::json data) {
        auto g_id = to_sf(data["guild_id"]);
        auto g_ptr = discord::utils::get(guilds, [g_id](auto &gld) { return gld->id == g_id; });
        g_ptr->emojis = from_json_array<discord::Emoji>(data["emojis"]);
        func_holder.call<events::guild_emojis_update>(futures, ready, *g_ptr);
    }

    void Bot::guild_integrations_update_event(nlohmann::json data) {
        auto g_id = to_sf(data["guild_id"]);
        func_holder.call<events::guild_integrations_update>(
            futures, ready, *discord::utils::get(guilds, [g_id](auto const &gld) { return gld->id == g_id; }));
    }

    void Bot::guild_member_add_event(nlohmann::json data) {
        snowflake guild_id = to_sf(data["guild_id"]);
        auto guild = discord::utils::get(guilds, [guild_id](auto &g) {
            return g->id == guild_id;
        });

        snowflake mem_id = to_sf(data["user"]["id"]);
        std::shared_ptr<discord::User> usr_ptr = discord::utils::get(users, [mem_id](auto const &usr) { return usr->id == mem_id; });
        if (!usr_ptr) {
            usr_ptr = std::make_shared<discord::User>(data["user"]);
            users.push_back(usr_ptr);
        }

        auto mem_ptr = std::make_shared<discord::Member>(data, guild);
        guild->members.push_back(mem_ptr);

        func_holder.call<events::guild_member_add>(futures, ready, *mem_ptr);
    }

    void Bot::guild_member_remove_event(nlohmann::json data) {
        auto usr_id = to_sf(data["user"]["id"]);
        auto usr_ptr = discord::utils::get(users, [usr_id](auto const &usr) {
            return usr->id == usr_id;
        });

        if (!usr_ptr) {
            usr_ptr = std::make_shared<discord::User>(data["user"]);
            users.push_back(usr_ptr);
        }

        auto g_id = to_sf(data["guild_id"]);
        auto guild = discord::utils::get(guilds, [g_id](auto &gld) { return gld->id == g_id; });

        guild->members.erase(std::remove_if(guild->members.begin(), guild->members.end(), [&usr_id](auto const &i) {
                                 return i->id == usr_id;
                             }),
                             guild->members.end());
        func_holder.call<events::guild_member_remove>(futures, ready, *guild, *usr_ptr);
    }

    void Bot::guild_member_update_event(nlohmann::json data) {
        auto usr_id = to_sf(data["user"]["id"]);

        snowflake guild_id = to_sf(data["guild_id"]);
        auto guild = discord::utils::get(this->guilds, [guild_id](auto &g) {
            return g->id == guild_id;
        });

        auto member = discord::utils::get(guild->members, [usr_id](auto &mem) { return mem->id == usr_id; });

        update_object(data, "nick", member->nick);
        if (data.contains("roles")) {
            member->roles.clear();
            for (auto const &each_id : data["roles"]) {
                member->roles.push_back(discord::utils::get(
                    guild->roles, [each_id](auto const &role) { return role->id == each_id; }));
            }
        }
        func_holder.call<events::guild_member_update>(futures, ready, *member);
    }

    // TODO: implement
    void Bot::guild_members_chunk_event(nlohmann::json) {
        // we don't do this.
    }

    void Bot::guild_role_create_event(nlohmann::json data) {
        auto g_id = to_sf(data["guild_id"]);
        auto guild = discord::utils::get(guilds, [=](auto &g) { return g->id == g_id; });
        auto role = std::make_shared<discord::Role>(data["role"], guild);
        guild->roles.push_back(role);
        func_holder.call<events::guild_role_create>(futures, ready, *role);
    }

    void Bot::guild_role_update_event(nlohmann::json data) {
        auto g_id = to_sf(data["guild_id"]);
        auto guild = discord::utils::get(guilds, [=](auto &g) { return g->id == g_id; });
        auto role_id = to_sf(data["role"]["id"]);
        auto role = discord::utils::get(guild->roles, [=](auto &rl) { return role_id == rl->id; });
        role->update(data);
        func_holder.call<events::guild_role_update>(futures, ready, *role);
    }

    void Bot::guild_role_delete_event(nlohmann::json data) {
        auto r_id = to_sf(data["role_id"]);
        auto g_id = to_sf(data["guild_id"]);
        auto guild = discord::utils::get(guilds, [=](auto &g) { return g_id == g->id; });
        for (size_t i = 0; i < guild->roles.size(); i++) {
            if (guild->roles[i]->id == r_id) {
                func_holder.call<events::guild_role_delete>(futures, ready, *guild->roles[i]);
                guild->roles.erase(guild->roles.begin() + i);
            }
        }
    }

    void Bot::message_create_event(nlohmann::json data) {
        auto message = std::make_shared<discord::Message>(data);
        if (messages.size() >= message_cache_count) {
            messages.erase(messages.begin());
        }
        messages.push_back(message);
        func_holder.call<events::message_create>(futures, ready, *message);
        fire_commands(*message);
    }

    void Bot::message_update_event(nlohmann::json data) {
        auto m_id = to_sf(data["id"]);
        for (std::size_t i = 0; i < messages.size(); i++) {
            if (messages[i]->id == m_id) {
                messages[i]->update(data);
                func_holder.call<events::message_update>(futures, ready, *messages[i]);
                break;
            }
        }
        func_holder.call<events::raw_message_update>(futures, ready, m_id, to_sf(data["channel_id"]), data);
    }

    void Bot::message_delete_event(nlohmann::json data) {
        auto m_id = to_sf(data["id"]);

        for (std::size_t i = 0; i < messages.size(); i++) {
            if (messages[i]->id == m_id) {
                auto return_m = messages[i];
                messages.erase(messages.begin() + i);
                func_holder.call<events::message_delete>(futures, ready, *return_m);
                break;
            }
        }

        func_holder.call<events::raw_message_delete>(futures, ready, m_id, to_sf(data["channel_id"]), data);
    }

    void Bot::message_delete_bulk_event(nlohmann::json data) {
        std::vector<discord::Message> r_vec;
        for (auto const &each_m : data["id"]) {
            auto m_id = to_sf(each_m);
            for (size_t i = 0; i < messages.size(); i++) {
                if (messages[i]->id == m_id) {
                    r_vec.push_back(*(messages[i]));
                    messages.erase(messages.begin() + i);
                    break;
                }
            }
        }

        func_holder.call<events::message_delete_bulk>(futures, !r_vec.empty(), r_vec);
        func_holder.call<events::raw_message_delete_bulk>(futures, ready, to_sf(data["channel_id"]), data);
    }

    void Bot::message_reaction_add_event(nlohmann::json data) {
        auto m_id = to_sf(data["message_id"]);
        auto message = discord::utils::get(messages, [=](auto &msg) { return msg->id == m_id; });
        if (!message) {
            message = std::make_shared<discord::Message>();
        }
        message->id = m_id;
        auto c_id = to_sf(data["channel_id"]);
        if (!message->channel) {
            message->channel = discord::utils::get(channels, [=](auto &chan) { return chan->id == c_id; });
        }
        message->channel->id = c_id;

        if (data.contains("guild_id") && !message->channel->guild) {
            auto g_id = to_sf(data["guild_id"]);
            message->channel->guild = discord::utils::get(guilds, [=](auto &gld) { return gld->id == g_id; });
        }
        func_holder.call<events::message_reaction_add>(futures, ready, *message, discord::Emoji{ data["emoji"] });
    }

    void Bot::message_reaction_remove_event(nlohmann::json data) {
        auto m_id = to_sf(data["message_id"]);
        auto message = discord::utils::get(messages, [=](auto &msg) { return msg->id == m_id; });
        if (!message) {
            message = std::make_shared<discord::Message>();
        }
        message->id = m_id;
        auto c_id = to_sf(data["channel_id"]);
        if (!message->channel) {
            message->channel = discord::utils::get(channels, [=](auto &chan) { return chan->id == c_id; });
        }
        message->channel->id = c_id;

        if (data.contains("guild_id") && !message->channel->guild) {
            auto g_id = to_sf(data["guild_id"]);
            message->channel->guild = discord::utils::get(guilds, [=](auto &gld) { return gld->id == g_id; });
        }
        func_holder.call<events::message_reaction_remove>(futures, ready, *message, discord::Emoji{ data["emoji"] });
    }

    void Bot::message_reaction_remove_all_event(nlohmann::json data) {
        auto m_id = to_sf(data["message_id"]);
        auto message = discord::utils::get(messages, [=](auto &msg) { return msg->id == m_id; });
        if (!message) {
            message = std::make_shared<discord::Message>();
        }
        message->id = m_id;
        auto c_id = to_sf(data["channel_id"]);
        if (!message->channel) {
            message->channel = discord::utils::get(channels, [=](auto &chan) { return chan->id == c_id; });
        }
        message->channel->id = c_id;

        if (data.contains("guild_id") && !message->channel->guild) {
            auto g_id = to_sf(data["guild_id"]);
            message->channel->guild = discord::utils::get(guilds, [=](auto &gld) { return gld->id == g_id; });
        }
        func_holder.call<events::message_reaction_remove_all>(futures, ready, *message);
    }

    void Bot::presence_update_event(nlohmann::json data) {
        auto g_id = to_sf(data["guild_id"]);
        auto mem_id = to_sf(data["user"]["id"]);
        auto guild = discord::utils::get(guilds, [=](auto &gld) { return gld->id == g_id; });
        if (!guild) {
            return;
        }
        auto member = discord::utils::get(guild->members, [=](auto &mem) { return mem->id == mem_id; });
        if (!member) {
            member = std::make_shared<discord::Member>(guild->get_member(mem_id).get());
            guild->members.push_back(member);
        }
        member->presence.update(data);
        func_holder.call<events::presence_update>(futures, ready, *member);
    }

    void Bot::typing_start_event(nlohmann::json data) {
        auto c_id = to_sf(data["channel_id"]);
        auto usr_id = to_sf(data["user_id"]);
        auto channel = discord::utils::get(channels, [=](auto const &c) { return c->id == c_id; });
        auto user = discord::utils::get(users, [=](auto const &usr) { return usr->id == usr_id; });
        func_holder.call<events::typing_start>(futures, ready, *user, *channel, boost::posix_time::from_time_t(time_t{ data["timestamp"].get<int>() }));
    }

    void Bot::user_update_event(nlohmann::json data) {
        auto usr_id = to_sf(data["id"]);
        auto user = discord::utils::get(users, [=](auto &usr) { return usr->id == usr_id; });
        user->update(data);
        func_holder.call<events::user_update>(futures, ready, *user);
    }

    void Bot::voice_state_update_event(nlohmann::json data) {
        func_holder.call<events::voice_state_update>(futures, ready, data);
    }

    void Bot::voice_server_update_event(nlohmann::json data) {
        func_holder.call<events::voice_server_update>(futures, ready, data);
    }

    void Bot::webhooks_update_event(nlohmann::json data) {
        auto c_id = to_sf(data["channel_id"]);
        auto channel = discord::utils::get(channels, [=](auto const &chan) { return chan->id == c_id; });
        func_holder.call<events::webhooks_update>(futures, ready, *channel);
    }

    pplx::task<discord::User> Bot::get_current_user() {
        /**
         * @brief Fetch the current user (bot) from the discord api
         * 
         * ```cpp
         *      std::cout << bot.get_current_user().get() << std::endl;
         * ```
         * 
         * @return pplx::task<discord::User> that will eventually yield a discord::User object, which is the current user, the bot.
         */
        return send_request(methods::GET, endpoint("/users/@me"), 0, global)
            .then([](pplx::task<Result<nlohmann::json>> const &resp) {
                return discord::User{ resp.get().unwrap() };
            });
    }

    pplx::task<discord::User> Bot::get_user(snowflake u_id) {
        /**
         * @brief Fetch a user (discord::User) from the discord api
         * 
         * ```cpp
         *      std::cout << bot.get_user(302517149359144962).get() << std::endl;
         * ```
         * 
         * @param[in] u_id the snowflake id of the user to fetch
         * 
         * @throws json Anything that discord::User's constructor can throw.
         * @return pplx::task<discord::User> that will eventually yield a discord::User object, which is the user with id \ref u_id
         */
        return send_request(methods::GET, endpoint("/users/%", u_id), 0, global)
            .then([](pplx::task<Result<nlohmann::json>> const &resp) {
                return discord::User{ resp.get().unwrap() };
            });
    }

    pplx::task<discord::User> Bot::edit(std::string const &username) {
        /**
         * @brief Edits the current user by making a call to the discord api
         * 
         * ```cpp
         *      bot.edit("New_username").wait();
         * ```
         * 
         * @param[in] username The new username of your bot.
         * 
         * Avatar is not supported as of right now, support might come soon.
         * 
         * @throws json Anything that discord::User's constructor can throw
         * @return pplx::task<discord::User> that will eventually yield the new discord::User object, which represents the current bot instance.
         */
        return send_request(methods::PATCH,
                            endpoint("/users/@me"), 0, global,
                            { { "username", username } })
            .then([](pplx::task<Result<nlohmann::json>> const &resp) {
                return discord::User{
                    resp.get().unwrap()
                };
            })
            .then([this](discord::User const &usr) {
                this->username = usr.name;
                this->avatar = usr.avatar;
                return usr;
            });
    }

    pplx::task<std::vector<discord::Guild>> Bot::get_user_guilds(int limit, snowflake before, snowflake after) {
        /**
         * @brief Gets \limit amount of guilds that the current bot user is in
         * 
         * Set before and after to the default value if you want any guild.
         * If limit is set to 0, 100 guilds will be yielded.
         * 
         * ```cpp
         *      for (auto const& guild : bot.get_guilds(150, 0, 0)) { 
         *          std::cout << guild << std::endl; 
         *      }
         * ```
         * 
         * @param[in] limit The amount of guilds to fetch at most.
         * @param[in] before If this is set, only guilds with a snowflake, id, lower than \ref before will be received.
         * @param[in] after If this is set, only guilds with a snowflake, id, higher than \ref after will be received.
         * 
         * @throws json Anything that discord::Guild's constructor can throw
         * @return pplx::task<std::vector<discord::Guild>> that will eventually yield the std::vector<discord::Guild> object, which contains the guilds that were just yielded.
         */

        nlohmann::json data({ { "limit", limit ? limit : 100 } });

        if (before) {
            data["before"] = before;
        }

        if (after) {
            data["after"] = after;
        }

        return send_request(methods::GET, endpoint("/users/@me/guilds"), 0, global, data)
            .then([this](pplx::task<Result<nlohmann::json>> const &resp) {
                std::vector<discord::Guild> g_vec{};
                for (auto const &each : resp.get().unwrap()) {
                    snowflake guild_id = to_sf(each["id"]);
                    g_vec.push_back(*discord::utils::get(this->guilds, [guild_id](auto const &guild) {
                        return guild->id == guild_id;
                    }));
                }
                return g_vec;
            });
    }

    pplx::task<discord::Channel> Bot::create_group_dm(std::vector<std::string> const &access_tokens, nlohmann::json const &nicks) {
        /**
         * @brief Creates a group DM.
         * 
         * 
         * @param[in] acces_tokens A vector of std::strings of the users you want to add their discord access_tokens.
         * @param[in] nicks A json dictionary representation of user snowflakes to their respective nicknames.
         * 
         * For example, a \ref nicks could be the following:
         * ```json
         *      {
         *          { 518149726294769676, "Luke" },
         *          { 553478921870508061, "Jake" }
         *      }
         * ```
         * 
         * @throws json Anything that discord::Channel's constructor can throw
         * @return pplx::task<discord::Channel> that will eventually yield the new discord::Channel object, which is the new dm_channel that was just created.
         */

        nlohmann::json data({ { "access_tokens", nlohmann::json::array() }, { "nicks", nicks } });
        for (auto const &each : access_tokens) {
            data["access_tokens"].push_back(each);
        }
        return send_request(methods::POST, endpoint("/users/@me/channels"), 0, global, data)
            .then([](pplx::task<Result<nlohmann::json>> const &resp) {
                return discord::Channel{ resp.get().unwrap() };
            });
    }

    pplx::task<std::vector<Bot::Connection>> Bot::get_connections() {
        /**
         * @brief Gets the bot its connections
         * 
         * ```cpp
         *      for (auto const& each : bot.get_connections().get()) {
         *         std::cout << each.name << " -> " << each.id << std::endl;
         *      }
         * ```
         * 
         * @throws json Anything that nlohmann::json can throw
         * @return pplx::task<std::vector<Connection>> that will eventually yield the the std::vector<Connection>, which contains the connections that your bot has.
         */
        return send_request(methods::GET,
                            endpoint("/users/@me/connections"),
                            0, global)
            .then([](pplx::task<Result<nlohmann::json>> const &resp) {
                std::vector<Connection> conn_vec;
                for (auto const &each : resp.get().unwrap()) {
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
            });
    }

    pplx::task<discord::Guild> Bot::get_guild(snowflake g_id) {
        /**
         * @brief Fetch a guild (discord::Guild) from the discord api
         * 
         * ```cpp
         *      std::cout << bot.get_guild(562636135428521984).get() << std::endl;
         * ```
         * 
         * @param[in] g_id The snowflake, id, of the guild to fetch.
         *
         * @throws json Anything that discord::Guild's constructor can throw.
         * @return pplx::task<discord::Guild> that will eventually yield a discord::Guild object, which is the Guild with id \ref g_id
         */
        return send_request(methods::GET, endpoint("/guilds/%", g_id), id, 0)
            .then([](pplx::task<Result<nlohmann::json>> const &resp) {
                return discord::Guild{ resp.get().unwrap() };
            });
    }

    pplx::task<discord::Channel> Bot::get_channel(snowflake chan_id) {
        /**
         * @brief Fetch a channel (discord::Channel) from the discord api
         * 
         * ```cpp
         *      std::cout << bot.get_channel(570591769302007838).get() << std::endl;
         * ```
         * 
         * @param[in] chan_id The snowflake, id, of the channel to fetch
         *
         * @throws json Anything that discord::Channel's constructor can throw.
         * @return pplx::task<discord::Channel> that will eventually yield a discord::Channel object, which is the channel with id \ref chan_id
         */
        return send_request(methods::GET, endpoint("/channels/%", chan_id), chan_id, channel)
            .then([](pplx::task<Result<nlohmann::json>> const &resp) {
                return discord::Channel{ resp.get().unwrap() };
            });
    }

    int Bot::wait_for_ratelimits(snowflake obj_id, int bucket_) {
        /**
         * @brief Internal functions used for not flooding the api by waiting for ratelimits.
         * 
         * @param[in] obj_id Object snowflake of the object that this ratelimit is for.
         * @param[in] bucket_ Bucket type of the snowflake, either channel, guild, webhook or global.
         * 
         * @return int the amount of seconds that have to be waited.
         */
        RateLimit *rlmt = nullptr;

        if (global_ratelimits.rate_limit_remaining == 0) {
            rlmt = &global_ratelimits;
        } else if (bucket_ == bucket_type::channel) {
            rlmt = &channel_ratelimits[obj_id];
        } else if (bucket_ == bucket_type::guild) {
            rlmt = &guild_ratelimits[obj_id];
        } else if (bucket_ == bucket_type::webhook) {
            rlmt = &webhook_ratelimits[obj_id];
        } else if (bucket_ == bucket_type::global) {
            rlmt = &global_ratelimits;
        } else {
            assert(!(std::string("Invalid bucket type in wait_for_ratelimits -> ") + std::to_string(bucket_)).empty());
        }

        if (rlmt->rate_limit_remaining == 0) {
            auto cur_time = boost::posix_time::second_clock::universal_time() - rlmt->ratelimit_reset;
            while (cur_time.is_negative()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                cur_time = boost::posix_time::second_clock::universal_time() - rlmt->ratelimit_reset;
            }
            return cur_time.seconds();
        }
        return 0;
    }

    void Bot::handle_ratelimits(web::http::http_headers &headers, snowflake obj_id, int bucket_) {
        /**
         * @brief Internal functions used for not flooding the api by updating ratelimit info.
         * 
         * @param[in] headers The headers that should be read for applying ratelimits.
         * @param[in] obj_id Object snowflake of the object that this ratelimit is for.
         * @param[in] bucket_ Bucket type of the snowflake, either channel, guild, webhook or global.
         * 
         * @return void
         */
        RateLimit *obj = nullptr;

        if (headers.has("X-RateLimit-Global")) {
            obj = &global_ratelimits;
        } else if (headers.has("X-RateLimit-Limit")) {
            if (bucket_ == bucket_type::channel) {
                obj = &channel_ratelimits[obj_id];
            } else if (bucket_ == bucket_type::guild) {
                obj = &guild_ratelimits[obj_id];
            } else if (bucket_ == bucket_type::webhook) {
                obj = &webhook_ratelimits[obj_id];
            } else if (bucket_ == bucket_type::global) {
                obj = &global_ratelimits;
            } else {
                assert(!(std::string("Invalid bucket type in handle_ratelimits -> ") + std::to_string(bucket_)).empty());
            }
        } else {
            return;
        }

        obj->rate_limit_limit = std::stoi(headers["X-RateLimit-Limit"]);
        obj->rate_limit_remaining = std::stoi(headers["X-RateLimit-Remaining"]);
        obj->ratelimit_reset = boost::posix_time::from_time_t(std::stoi(headers["X-RateLimit-Reset"]));
    }
}  // namespace discord