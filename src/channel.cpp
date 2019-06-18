#include "channel.hpp"
#include "bot.hpp"
#include "emoji.hpp"
#include "invite.hpp"
#include "message.hpp"
#include "utils.hpp"
#include "webhook.hpp"

discord::Channel::Channel(snowflake id)
    : discord::Object(id) {
    /**
     * @brief constructs a discord::Channel from a snowflake id
     * 
     * ```cpp
     *      discord::Channel{ 570591769302007838 }.send("Hello!").wait();
     * ```
     * 
     * @param[in] id Snowflake of the channel to get
     * 
     * The only field that will be populated is \ref id if the channel cannot be retrieved from cache, however, messages can still be sent here.
     * Accessing any other fields of this object will result in undefined behavior
     */
    auto c = discord::utils::get(discord::detail::bot_instance->channels, [id](auto const &c) {
        return c->id == id;
    });
    if (!c) {
        return;
    }
    *this = *c;
}

discord::Channel::Channel(nlohmann::json const data, snowflake guild_id)
    : discord::Object(to_sf(data["id"].get<std::string>())) {
    /**
     * @brief Constructs a channel from raw event data
     * 
     * @param[in] data Raw event data
     * @param[in] guild_id Snowflake of the guild this channel belongs to, set to 0 if it doesn't belong to a guild
     * 
     */
    type = get_value(data, "type", 0);
    bitrate = get_value(data, "bitrate", 0);
    user_limit = get_value(data, "user_limit", 0);
    auto parent_id = to_sf(get_value(data, "parent_id", "0"));
    parent = discord::utils::get(discord::detail::bot_instance->channels, [parent_id](auto const &chn) { return chn->id == parent_id; });
    rate_limit_per_user = get_value(data, "rate_limit_per_user", 0);
    topic = get_value(data, "topic", "");

    if (guild_id) {
        guild = discord::utils::get(discord::detail::bot_instance->guilds, [guild_id](auto const &g) {
            return g->id == guild_id;
        });
    }

    if (data.contains("recipients")) {
        for (auto const &each : data["recipients"]) {
            recipients.emplace_back(each);
        }
    }

    if (data.contains("permission_overwrites")) {
        for (auto &each : data["permission_overwrites"]) {
            int t = each["type"].get<std::string>() != "role";
            overwrites.emplace_back(each["allow"].get<int>(),
                                    each["deny"].get<int>(),
                                    to_sf(each["id"]),
                                    t);
        }
    }
    name = get_value(data, "name", "");
    position = get_value(data, "position", 0);
    id = to_sf(get_value(data, "id", "0"));
}

discord::Channel &discord::Channel::update(nlohmann::json const data) {
    /**
     * @brief Updates a channel after getting a channel_update event
     * 
     * @param[in] data Raw channel_update event data
     * 
     * This function should NOT be used by a user
     */
    update_object(data, "id", id);
    update_object(data, "bitrate", bitrate);
    update_object(data, "user_limit", user_limit);
    if (data.contains("parent_id")) {
        auto parent_id = to_sf(get_value(data, "parent_id", "0"));
        parent = discord::utils::get(discord::detail::bot_instance->channels, [parent_id](auto const &chn) { return chn->id == parent_id; });
    }
    update_object(data, "rate_limit_per_user", rate_limit_per_user);
    update_object(data, "topic", topic);
    update_object(data, "name", name);
    update_object(data, "position", position);
    update_object(data, "type", type);

    if (type == channel_type::dm_channel || type == channel_type::group_dm_channel) {
        if (data.contains("recipients")) {
            recipients.clear();
            for (auto const &each : data["recipients"]) {
                recipients.emplace_back(each);
            }
        }
    }

    return *this;
}

pplx::task<discord::Message> discord::Channel::send(std::string const &content, std::vector<File> const &files, bool tts) const {
    /**
     * @brief Sends a message to this channel
     * 
     * @param[in] content The content, string, of the message
     * @param[in] files A vector of discord::File objects to be sent to the channel
     * @param[in] tts Whether this message should be sent with text-to-speech
     * 
     * ```cpp
     *      channel.send("Hello!", {}, false).wait();
     *      channel.send("Hello, this is todo.txt, sent as text to speech: ", { { "todo.txt", "todo.txt", false } }, true).wait();
     * ```
     * 
     * If content is set to "", but files is set, then only files will be sent.
     * 
     * @throws If content is set to "", and files is not set, an exception will be thrown.
     * @throws If content is longer than 2000 characters, an std::runtime_error or json parser error will be thrown.
     * @throws If your bot is unauthorized to send a message to this channel, an std::runtime_error or json parser error will be thrown
     * 
     * @return pplx::task<discord::Message> which eventually yields the sent discord::Message object.
     */
    discord::detail::bot_instance->wait_for_ratelimits(id, channel);
    std::vector<std::pair<std::ifstream, std::string>> pass_data = {};
    for (auto const &each : files) {
        std::string custom_filename = each.spoiler ? "SPOILER_" : "";
        if (is_image_or_gif(each.filepath)) {
            pass_data.emplace_back(std::ifstream{ each.filepath, std::ios::binary }, custom_filename + each.filename);
        } else {
            pass_data.emplace_back(std::ifstream{ each.filepath }, custom_filename + each.filename);
        }
    }

    auto p = generate_form_data(pass_data, nlohmann::json{
                                               { "content", content },
                                               { "tts", tts } });

    web::http::client::http_client client{ { endpoint("/channels/%/messages", this->id) } };
    web::http::http_request msg{ methods::POST };


    msg.set_body(p.second, "multipart/form-data; boundary=" + p.first);
    msg.headers().add("Authorization", format("Bot %", discord::detail::bot_instance->token));
    msg.headers().add("User-Agent", "DiscordBot (http://www.github.com/yuhanun/dpp, 0.0.0)");
    msg.headers().add("Accept", "*/*");

    auto send_lambda = [&]() {
        return client.request(msg).then([=](http_response resp_val) {
            discord::detail::bot_instance->handle_ratelimits(resp_val.headers(), this->id, channel);
            auto next_action = static_cast<int>(handle_resp(resp_val.status_code()));

            auto response = resp_val.extract_utf8string(true).get();
            auto parsed_json = response.empty() ? nlohmann::json{} : nlohmann::json::parse(response);

            if (next_action == success) {
                return OK(parsed_json);
            } else if (next_action == resend_request) {
                return ERR<nlohmann::json>("", {}, {}, resp_val);
            } else {
                return ERR<nlohmann::json>(get_value(parsed_json, "message", ""), {}, get_default_headers(), resp_val);
            }
        });
    };
    return send_lambda().then([&](pplx::task<Result<nlohmann::json>> task) {
        while (!task.get().is_ok()) {
            if (task.get().unwrap_err().response.status_code() == 429) {
                task = send_lambda();
            }
        }
        return discord::Message{ task.get().unwrap() };
    });
}

pplx::task<discord::Message> discord::Channel::send(EmbedBuilder const &embed, std::vector<File> const &files, bool tts, std::string const &content) const {
    /**
     * @brief Sends a message to this channel
     * 
     * @param[in] embed The embed, which should be sent
     * @param[in] files A vector of discord::File objects to be sent to the channel
     * @param[in] tts Whether this message should be sent with text-to-speech
     * @param[in] content The additional content of the message that should be sent
     * 
     * ```cpp
     *      channel.send(
     *          discord::EmbedBuilder().set_title("Hello!"), 
     *          { { "todo.txt", "todo.txt", false } }, 
     *          true, "Hello, this is todo.txt, sent as text to speech: ").wait();
     * ```
     * 
     * 
     * @throws If none of \ref files, \ref embed and \ref content have been set, then this will throw an std::runtime_error
     * @throws If \ref content or \ref embed exceeds limit, an exception will be thrown
     * @throws If your bot is unauthorized to send a message to this channel, an std::runtime_error or json parser error will be thrown
     * 
     * @return pplx::task<discord::Message> which eventually yields the sent discord::Message object.
     */
    discord::detail::bot_instance->wait_for_ratelimits(id, channel);
    std::vector<std::pair<std::ifstream, std::string>> pass_data = {};
    for (auto const &each : files) {
        std::string custom_filename = each.spoiler ? "SPOILER_" : "";
        if (is_image_or_gif(each.filepath)) {
            pass_data.emplace_back(std::ifstream{ each.filepath, std::ios::binary }, custom_filename + each.filename);
        } else {
            pass_data.emplace_back(std::ifstream{ each.filepath }, custom_filename + each.filename);
        }
    }

    auto p = generate_form_data(pass_data, nlohmann::json{
                                               { "content", content },
                                               { "tts", tts },
                                               { "embed", embed.to_json() } });

    web::http::client::http_client client{ { endpoint("/channels/%/messages", this->id) } };
    web::http::http_request msg{ methods::POST };

    msg.set_body(p.second, "multipart/form-data; boundary=" + p.first);
    msg.headers().add("Authorization", format("Bot %", discord::detail::bot_instance->token));
    msg.headers().add("User-Agent", "DiscordBot (http://www.github.com/yuhanun/dpp, 0.0.0)");
    msg.headers().add("Accept", "*/*");

    auto send_lambda = [&]() {
        return client.request(msg).then([=](http_response resp_val) {
            discord::detail::bot_instance->handle_ratelimits(resp_val.headers(), this->id, channel);
            auto next_action = static_cast<int>(handle_resp(resp_val.status_code()));

            auto response = resp_val.extract_utf8string(true).get();
            auto parsed_json = response.empty() ? nlohmann::json{} : nlohmann::json::parse(response);

            if (next_action == success) {
                return OK(parsed_json);
            } else if (next_action == resend_request) {
                return ERR<nlohmann::json>("", {}, {}, resp_val);
            } else {
                return ERR<nlohmann::json>(get_value(parsed_json, "message", ""), {}, get_default_headers(), resp_val);
            }
        });
    };
    return send_lambda().then([&](pplx::task<Result<nlohmann::json>> task) {
        while (!task.get().is_ok()) {
            if (task.get().unwrap_err().response.status_code() == 429) {
                task = send_lambda();
            }
        }
        return discord::Message{ task.get().unwrap() };
    });
}

pplx::task<void> discord::Channel::bulk_delete(std::vector<discord::Message> &m) {
    /**
     * @brief Bulk deletes the messsages passed through \ref m
     * 
     * @param[in] m an std::vector of message objects, these messages only need to have their `id` field, any other field is optional for this purpose.
     * 
     * ```cpp
     *      auto messages = channel.get_messages(50).get();
     *      channel.bulk_delete(messages).wait();
     * ```
     * 
     * @throws Throws an std::runtime_error if your token is unauthorized.
     * @return pplx::task<void> Which will not yield a result, but should be `wait`'ed regardless.
     */
    nlohmann::json array = nlohmann::json::array();
    for (auto const &each : m) {
        array.push_back(each.id);
    }
    nlohmann::json data = nlohmann::json();
    data["messages"] = array;
    return send_request(methods::POST, endpoint("/channels/%/messages/bulk-delete", id), id, channel, data)
        .then([](pplx::task<Result<nlohmann::json>> const) {});
}

pplx::task<std::vector<discord::Message>> discord::Channel::get_messages(int limit) {
    /**
     * @brief Fetches \ref limit messages from the current channel
     * 
     * @param[in] limit The amount of messages to fetch, should not be negative.
     * 
     * ```cpp
     *      auto messages = channel.get_messages(50).get();
     *      for (auto const& message : messages) {
     *          std::cout << message.content << std::endl;
     *      }
     * ```
     * 
     * @throws Throws an std::runtime_error if your token is unauthorized to get messages from this chanenl.
     * @return pplx::task<std::vector<discord::Message>> which eventually yields an std::vector<discord::Message> containing the messages which have just been fetched.
     */
    return send_request(methods::GET,
                        endpoint("/channels/%/messages?limit=%", id, limit),
                        id,
                        channel)
        .then([](pplx::task<Result<nlohmann::json>> const &resp) {
            return from_json_array<discord::Message>(resp.get().unwrap());
        });
}

pplx::task<discord::Channel> discord::Channel::edit(nlohmann::json &j) {
    /**
     * @brief Edits the current channel
     * 
     * @param[in] j Takes an nlohmann::json which may contain any of the fields below.
     * 
     * name (string) 2-100 character channel name  <br>
     * position (integer) The position of the channel in the left-hand listing  <br>
     * topic (string) 0-1024 character channel topic  <br>
     * nsfw (bool) Whether the cahnenl is NSFW  <br>
     * rate_limit_per_user (integer) Amount of seconds a user has to wait before sending another message (0-21600)  <br>
     * bitrate (integer) The bitrate (in bits) of the voice channel (8000 - 96000, 128000 for VIP servers)  <br>
     * user_limit (integer) The user limit of the voice channel, 0 is no limit, 1-99 refers to a limit<br>
     * permission_overwrites (array of discord::Overwrite objects) Channel specific permissions, you should .to_json() these.<br>
     * parent_id (snowflake) The ID of the new parent category for a channel<br>
     * 
     * ```cpp
     *      auto channel = channel.edit({
     *          {"name", "new_name"},
     *          {"position", 5},
     *          {"topic", "This is the new channel topic!"},
     *          {"nsfw", false}
     *      }).get().send("Updated channel!");
     * ```
     * 
     * @throws Throws an std::runtime_error if your token is unauthorized to edit this channel
     * @return pplx::task<discord::Channel> which eventually yields the updated channel object.
     */
    return send_request(methods::PATCH, endpoint("/channels/%", id), id, channel, j)
        .then([this](pplx::task<Result<nlohmann::json>> const &resp) {
            return discord::Channel{
                resp.get().unwrap(), this->id
            };
        });
}

pplx::task<void> discord::Channel::remove() {
    /**
     * @brief Deletes a channel
     * 
     * ```cpp
     *      channel.remove().wait();
     * ```
     * 
     * @throws Throws an std::runtime_error if your token is unauthorized to delete this channel
     * @return pplx::task<void> which doesn't yield anything, but should be `wait`'ed regardless
     */
    return send_request(methods::DEL, endpoint("/channels/%", id), id, channel)
        .then([](pplx::task<Result<nlohmann::json>> const &) {});
}

pplx::task<discord::Message> discord::Channel::get_message(snowflake id_) {
    /**
     * @brief Fetches a message with a specific snowflake, id, from the server
     * 
     * @param[in] id_ The snowflake, id, of the message to fetch.
     * 
     * ```cpp
     *      discord::Message msg = channel.get_message(582664462604173322).get();
     * ```
     * 
     * @throws Throws an std::runtime_error if your token is unauthorized to read messages in this channel
     * @return pplx::task<discord::Message> which eventually yields the fetched discord::Message object
     */
    return send_request(methods::GET,
                        endpoint("/channels/%/messages/%", this->id, id_), this->id, channel)
        .then([](pplx::task<Result<nlohmann::json>> const &resp) {
            return discord::Message{ resp.get().unwrap() };
        });
}

pplx::task<std::vector<discord::Invite>> discord::Channel::get_invites() {
    /**
     * @brief Fetches all invites created for this specific channel
     * 
     * ```cpp
     *      std::vector<discord::Invite> invites = channel.get_invites().get();
     *      for (auto const& each : invites) {
     *         std::cout << each.code << std::endl; 
     *       }
     * ```
     * @throws Throws an std::runtime_error if your token is unauthorized to get invites from this channel
     * @return pplx::task<std::vector<discord::Invite>> which eventually yields the fetched invites.
     */
    return send_request(methods::GET,
                        endpoint("/channels/%/invites", id),
                        id, channel)
        .then([](pplx::task<Result<nlohmann::json>> const &resp) {
            return from_json_array<discord::Invite>(resp.get().unwrap());
        });
}

pplx::task<discord::Invite> discord::Channel::create_invite(int max_age, int max_uses, bool temporary, bool unique) const {
    /**
     * @brief Creates an invite for the current channel
     * 
     * @param[in] max_age Duration of invites in secuonds before expiry, or 0 for never
     * @param[in] max_uses Max numbers of uses or 0 for unlimited
     * @param[in] temporary Whether this invite only grants temporary membership
     * @param[in] if true, don't try to reuse a similar invite
     * 
     * ```cpp
     *      auto invite = discord::format("discord.gg/%", channel.create_invite(3600, 1, false, true).get().code);
     * ```
     * 
     * @throws Throws an std::runtime_error if your token is unauthorized to create an invite to this channel
     * @return pplx::task<discord::Invite> which eventually yields your created invite.
     */
    return send_request(
               methods::POST,
               endpoint("/channels/%/invites", id),
               id, channel,
               { { "max_age", max_age },
                 { "max_uses", max_uses },
                 { "temporary", temporary },
                 { "unique", unique } })
        .then([](pplx::task<Result<nlohmann::json>> const &resp) {
            return discord::Invite{ resp.get().unwrap() };
        });
}

pplx::task<std::vector<discord::Message>> discord::Channel::get_pins() {
    /**
     * @brief Fetches all the pinned messages from the server
     * 
     * ```cpp
     *      for (auto const& each : channel.get_pins().get()) {
     *          std::cout << each.content << std::endl;
     *      }
     * ```
     * 
     * @throws Throws an std::runtime_error if your token is unuathorized to read messages in this channel
     * @return pplx::task<std::vector<discord::Invite>> which will eventually yield an std::vector containing the pinned messages.
     */
    return send_request(methods::GET,
                        endpoint("/channels/%/pins", id),
                        id, channel)
        .then([](pplx::task<Result<nlohmann::json>> const &resp) {
            return from_json_array<discord::Message>(resp.get().unwrap());
        });
}

pplx::task<discord::Webhook> discord::Channel::create_webhook(std::string const &name) {
    /**
     * @brief Creates a webhook for the current channel
     * 
     * ```cpp
     *      auto new_webhook = channel.create_webhook("MyCustomWebhook").get();
     *      new_webhook.send("I'm a custom webhook!").wait();
     * ```
     * 
     * @param[in] name The name of the webhook you're going to create
     * 
     * @throws Throws an std::runtime_error if your token is unauthorized to create webhooks in this channel
     * @return pplx::task<discord::Webhook> which will eventually yield your created webhook
     */
    return send_request(methods::POST,
                        endpoint("/channels/%/webhooks", id),
                        id, channel,
                        { { "name", name } })
        .then([](pplx::task<Result<nlohmann::json>> const &resp) {
            return discord::Webhook{ resp.get().unwrap() };
        });
}

pplx::task<std::vector<discord::Webhook>> discord::Channel::get_webhooks() {
    /**
     * @brief Fetches the current channel's webhooks
     * 
     * ```cpp
     *      auto webhooks = channel.get_webhooks().get();
     *      for (auto const& webhook : webhooks) {
     *          webhook.send("Hello!").wait();    
     *      }
     * ```
     * 
     * @throws an std::runtime_error if your token is unauthorized to get webhooks from this channel
     * @return pplx::task<std::vector<discord::Webhook>> which will eventually yield an std::vector containing this channel's webhooks
     */
    return send_request(methods::GET, endpoint("/channels/%/webhooks", id), id, channel)
        .then([](pplx::task<Result<nlohmann::json>> const &resp) {
            return from_json_array<discord::Webhook>(resp.get().unwrap());
        });
}

pplx::task<void> discord::Channel::remove_permissions(discord::Object const &obj) {
    /**
     * @brief Returns permissions from this channel for an object
     * 
     * @param[in] obj The object for which to remove the permissions
     * 
     * ```cpp
     *      channel.remove_permissions(discord::Object{ 201837409954955265 }).wait();
     * ```
     * 
     * @throws an std::runtime_error if your token is unauthorized to edit permissions for this channel
     * @return pplx::task<void> which will not yield anything but should be `wait`'ed regardless
     */
    return send_request(methods::DEL,
                        format("%/channels/%/permissions/%", id, obj.id),
                        id, channel)
        .then([](pplx::task<Result<nlohmann::json>> const &) {});
}

pplx::task<void> discord::Channel::typing() {
    /**
     * @brief Triggers `typing` status for your user in this channel
     * 
     * ```cpp
     *      channel.typing();
     *      auto calc = some_expensive_calculation();
     *      channel.send(discord::format("The result of the calculation is %", calc)).wait();
     * ```
     * 
     * @throws an std::runtime_error if your token is unauthorized to type in this channel
     * @return pplx::task<void> which will not yield anything but should be `wait`'ed regardless
     */
    return send_request(methods::POST,
                        endpoint("/channels/%/typing", id),
                        id, channel)
        .then([](pplx::task<Result<nlohmann::json>> const &) {});
}

pplx::task<void> discord::Channel::add_group_dm_recipient(discord::User const &user, std::string const &access_token, std::string const &nick) {
    /**
     * @brief Add a group recipient to the current DM group channel
     * 
     * This route cannot be used by bots!
     * 
     * @param[in] user User to add to the group DM.
     * @param[in] access_token Access token that should be used to add this user.
     * @param[in] nick Nickname of the user to add to the DM channel.
     * 
     * @throws an std::runtime_error if your token is unauthorized to add a group dm recipient.
     * @return pplx::task<void> which will not yield anything but should be `wait`'ed regardless
     */
    return send_request(methods::PUT,
                        endpoint("/channels/%/recipient/%", this->id, user.id),
                        id, channel, { { "access_token", access_token }, { "nick", nick } })
        .then([](pplx::task<Result<nlohmann::json>> const &) {});
}

pplx::task<void> discord::Channel::remove_group_dm_recipient(discord::User const &user) {
    /**
     * @brief Removes a user from the current DM group channel
     * 
     * This route cannot be used by bots!
     * 
     * @param[in] user User that should be removed from the group DM
     * 
     * @throws an std::runtime_error if your token is unauthorized to remove a group dm recipient.
     * @return pplx::task<void> which will not yield anything but should be `wait`'ed regardless
     */
    return send_request(methods::DEL,
                        endpoint("/channels/%/recipient/%", this->id, user.id),
                        id, channel)
        .then([](pplx::task<Result<nlohmann::json>> const &) {});
}

pplx::task<void> discord::Channel::edit_position(int new_pos) {
    /**
     * @brief Edits position of the current channel
     * 
     * ```cpp
     *      channel.edit_position(0).wait();
     * ```
     * 
     * @param[in] new_pos New position of the current channel
     * 
     * @throws an std::runtime_error if your token is unauthorized to edit this channel
     * @return pplx::task<void> which will not yield anything but should be `wait`'ed regardless
     */
    return send_request(methods::PATCH,
                        endpoint("/guilds/%/channels", this->guild->id),
                        this->guild->id, bucket_type::guild,
                        { { "id", this->id }, { "position", new_pos } })
        .then([](pplx::task<Result<nlohmann::json>> const &) {});
}
