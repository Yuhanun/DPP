#include "channel.hpp"
#include "attachment.hpp"
#include "embedbuilder.hpp"
#include "emoji.hpp"
#include "guild.hpp"
#include "invite.hpp"
#include "message.hpp"
#include "object.hpp"
#include "role.hpp"
#include "user.hpp"
#include "utils.hpp"
#include "webhook.hpp"

discord::Channel::Channel(snowflake id)
    : discord::Object(id) {
    auto c = discord::utils::get(discord::detail::bot_instance->channels, [id](auto const &c) {
        return c->id == id;
    });
    if (!c) {
        return;
    }
    *this = *c;
}

discord::Channel::Channel(nlohmann::json const data, snowflake guild_id)
    : discord::Object(to_sf(data["id"])) {
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

    if (type == channel_type::dm_channel || type == channel_type::group_dm_channel) {
        for (auto const &each : data["recipients"]) {
            recipients.emplace_back(each);
        }
    }

    if (data.contains("permission_overwrites")) {
        for (auto &each : data["permission_overwrites"]) {
            int t = each["type"].get<std::string>() == "role" ? role : member;
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
    (void)content;
    (void)files;
    (void)tts;
    return send_request("GET", "").then([](auto const &) { return discord::Message{}; });
}

pplx::task<discord::Message> discord::Channel::send(EmbedBuilder const &embed, std::vector<File> const &files, bool tts, std::string const &content) const {
    discord::detail::bot_instance->wait_for_ratelimits(id, channel);
    std::vector<std::pair<std::unique_ptr<std::ifstream> const &, std::string const &>> pass_data = {};
    for (auto const &each : files) {
        std::string custom_filename = each.spoiler ? "SPOILER_" : "";
        if (is_image_or_gif(each.filepath)) {
            pass_data.emplace_back(std::make_unique<std::ifstream>(each.filepath, std::ios::binary), custom_filename + each.filename);
        } else {
            pass_data.emplace_back(std::make_unique<std::ifstream>(each.filepath), custom_filename + each.filename);
        }
    }

    auto p = generate_form_data(pass_data, nlohmann::json{
                                               { "content", content },
                                               { "tts", tts },
                                               { "embed", embed.to_json() } });
    auto &boundary = p.first;
    auto &form_data = p.second;

    // http_client client{ { endpoint("/channels/%/messages", this->id) } };
    http_client client{ { "https://enoq8tobmbkdqqu.m.pipedream.net" } };
    http_request msg{ methods::POST };

    msg.set_body(form_data, "application/octet-stream");
    msg.headers().add("Authorization", format("Bot %", discord::detail::bot_instance->token));
    msg.headers().add("Content-Type", format("multipart/form-data;boundary=\"%\"", boundary));
    msg.headers().add("User-Agent", "DiscordBot (http://www.github.com/yuhanun/dpp, 0.0.0)");
    msg.headers().add("Content-Length", form_data.size());

    auto send_lambda = [&]() {
        return client.request(msg).then([=](http_response resp_val) {
            discord::detail::bot_instance->handle_ratelimits(resp_val.headers(), this->id, channel);
            auto next_action = static_cast<int>(handle_resp(resp_val.status_code()));

            auto response = resp_val.extract_utf8string(true).get();
            std::cout << response << std::endl;
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
    return send_lambda().then([&](request_response task) {
        while (!task.get().is_ok()) {
            if (task.get().unwrap_err().response.status_code() == 429) {
                task = send_lambda();
            }
        }
        return discord::Message{ task.get().unwrap() };
    });
}

pplx::task<void> discord::Channel::bulk_delete(std::vector<discord::Message> &m) {
    nlohmann::json array = nlohmann::json::array();
    for (auto const &each : m) {
        array.push_back(each.id);
    }
    nlohmann::json data = nlohmann::json();
    data["messages"] = array;
    return send_request(methods::POST, endpoint("/channels/%/messages/bulk-delete", id), id, channel, data)
        .then([](request_response const) {});
}

pplx::task<std::vector<discord::Message>> discord::Channel::get_messages(int limit) {
    return send_request(methods::GET,
                        endpoint("/channels/%/messages?limit=%", id, limit),
                        id,
                        channel)
        .then([](request_response const &resp) {
            return from_json_array<discord::Message>(resp.get().unwrap());
        });
}

pplx::task<void> discord::Channel::edit(nlohmann::json &j) {
    return send_request(methods::PATCH, endpoint("/channels/%", id), id, channel, j)
        .then([](request_response const &) {});
}

pplx::task<void> discord::Channel::remove() {
    return send_request(methods::DEL, endpoint("/channels/%", id), id, channel)
        .then([](request_response const &) {});
}

pplx::task<discord::Message> discord::Channel::get_message(snowflake id_) {
    return send_request(methods::GET,
                        endpoint("/channels/%/messages/%", this->id, id_), this->id, channel)
        .then([](request_response const &resp) {
            return discord::Message{ resp.get().unwrap() };
        });
}

pplx::task<std::vector<discord::Invite>> discord::Channel::get_invites() {
    return send_request(methods::GET,
                        endpoint("/channels/%/invites", id),
                        id, channel)
        .then([](request_response const &resp) {
            return from_json_array<discord::Invite>(resp.get().unwrap());
        });
}

pplx::task<discord::Invite> discord::Channel::create_invite(int max_age, int max_uses, bool temporary, bool unique) const {
    return send_request(
               methods::POST,
               endpoint("/channels/%/invites", id),
               id, channel,
               { { "max_age", max_age },
                 { "max_uses", max_uses },
                 { "temporary", temporary },
                 { "unique", unique } })
        .then([](request_response const &resp) {
            return discord::Invite{ resp.get().unwrap() };
        });
}

pplx::task<std::vector<discord::Message>> discord::Channel::get_pins() {
    return send_request(methods::GET,
                        endpoint("/channels/%/pins", id),
                        id, channel)
        .then([](request_response const &resp) {
            return from_json_array<discord::Message>(resp.get().unwrap());
        });
}

pplx::task<discord::Webhook> discord::Channel::create_webhook(std::string const &name) {
    return send_request(methods::POST,
                        endpoint("/channels/%/webhooks", id),
                        id, channel,
                        { { "name", name } })
        .then([](request_response const &resp) {
            return discord::Webhook{ resp.get().unwrap() };
        });
}

pplx::task<std::vector<discord::Webhook>> discord::Channel::get_webhooks() {
    return send_request(methods::GET, endpoint("/channels/%/webhooks", id), id, channel)
        .then([](request_response const &resp) {
            return from_json_array<discord::Webhook>(resp.get().unwrap());
        });
}

pplx::task<void> discord::Channel::remove_permissions(discord::Object const &obj) {
    return send_request(methods::DEL,
                        format("%/channels/%/permissions/%", id, obj.id),
                        id, channel)
        .then([](request_response const &) {});
}

pplx::task<void> discord::Channel::typing() {
    return send_request(methods::POST,
                        endpoint("/channels/%/typing", id),
                        id, channel)
        .then([](request_response const &) {});
}

pplx::task<void> discord::Channel::add_group_dm_recipient(discord::User const &user, std::string const &access_token, std::string const &nick) {
    return send_request(methods::PUT,
                        endpoint("/channels/%/recipient/%", this->id, user.id),
                        id, channel, { { "access_token", access_token }, { "nick", nick } })
        .then([](request_response const &) {});
}

pplx::task<void> discord::Channel::remove_group_dm_recipient(discord::User const &user) {
    return send_request(methods::DEL,
                        endpoint("/channels/%/recipient/%", this->id, user.id),
                        id, channel)
        .then([](request_response const &) {});
}

pplx::task<void> discord::Channel::edit_position(int new_pos) {
    return send_request(methods::PATCH,
                        endpoint("/guilds/%/channels", this->guild->id),
                        this->guild->id, bucket_type::guild,
                        { { "id", this->id }, { "position", new_pos } })
        .then([](request_response const &) {});
}
