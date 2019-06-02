#pragma once
#include <cpr/cpr.h>
#include "discord.hpp"
#include "invite.hpp"
#include "permissions.hpp"
#include "utility.hpp"

discord::Channel::Channel(snowflake id) {
    this->id = id;
    auto c = discord::utils::get(discord::detail::bot_instance->channels, [id](auto const &c) {
        return c->id == id;
    });
    if (!c) {
        return;
    }
    *this = *c;
}

discord::Channel::Channel(nlohmann::json const data, snowflake guild_id) {
    type = get_value(data, "type", 0);
    bitrate = get_value(data, "bitrate", 0);
    user_limit = get_value(data, "user_limit", 0);
    parent_id = to_sf(get_value(data, "parent_id", "0"));
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

discord::Message discord::Channel::send(std::string const &content, std::vector<File> const &files, bool tts) const {
    cpr::Multipart multipart_data{};

    for (size_t i = 0; i < files.size(); i++) {
        multipart_data.parts.emplace_back("file", cpr::File{ files[i].file }, "application/octet-stream");
    }

    auto payload_json = nlohmann::json{
        { "content", content },
        { "tts", tts }
    }.dump();
    multipart_data.parts.emplace_back("payload_json", payload_json);

    auto response = cpr::Post(
                        cpr::Url(get_channel_link(id)),
                        cpr::Header{ { "Authorization", format("Bot %", discord::detail::bot_instance->token) },
                                     { "Content-Type", "multipart/form-data" },
                                     { "User-Agent", "DiscordPP (http://www.github.com/yuhanun/dpp, 0.0.0)" },
                                     { "Connection", "keep-alive" } },
                        multipart_data)
                        .text;

#ifdef __DPP_DEBUG
    std::cout << response << std::endl;
#endif

    return discord::Message{ nlohmann::json::parse(response) };
}

discord::Message discord::Channel::send(EmbedBuilder const &embed, std::vector<File> const &files, bool tts, std::string const &content) const {
    cpr::Multipart multipart_data{};

    for (size_t i = 0; i < files.size(); i++) {
        multipart_data.parts.emplace_back("file", cpr::File{ files[i].file }, "application/octet-stream");
    }

    multipart_data.parts.emplace_back("payload_json", nlohmann::json{ { "content", content }, { "tts", tts }, { "embed", embed.to_json() } }.dump());

    auto response = cpr::Post(
                        cpr::Url(get_channel_link(id)),
                        cpr::Header{ { "Authorization", format("Bot %", discord::detail::bot_instance->token) },
                                     { "Content-Type", "multipart/form-data" },
                                     { "User-Agent", "DiscordPP (http://www.github.com/yuhanun/dpp, 0.0.0)" },
                                     { "Connection", "keep-alive" } },
                        multipart_data)
                        .text;

#ifdef __DPP_DEBUG
    std::cout << response << std::endl;
#endif

    return discord::Message{ nlohmann::json::parse(response) };
}

void discord::Channel::bulk_delete(std::vector<discord::Message> &m) {
    nlohmann::json array = nlohmann::json::array();
    for (auto const &each : m) {
        array.push_back(each.id);
    }
    nlohmann::json data = nlohmann::json();
    data["messages"] = array;
    send_request<request_method::Post>(data, get_default_headers(), endpoint("/channels/%/messages/bulk-delete", id));
}

std::vector<discord::Message> discord::Channel::get_messages(int limit) {
    std::vector<discord::Message> return_vec;

    auto data = send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), endpoint("/channels/%/messages?limit=%", id, limit));
    for (auto &each : data) {
        return_vec.emplace_back(each);
    }
    return return_vec;
}

void discord::Channel::edit(nlohmann::json &j) {
    send_request<request_method::Patch>(j, get_default_headers(), endpoint("/channels/%", id));
}

void discord::Channel::remove() {
    send_request<request_method::Delete>(nlohmann::json({}), get_default_headers(), endpoint("/channels/%", id));
}

discord::Message discord::Channel::get_message(snowflake id) {
    return discord::Message{
        send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), endpoint("/channels/%/messages/%", this->id, id))
    };
}

std::vector<discord::Invite> discord::Channel::get_invites() {
    std::vector<discord::Invite> return_vec;
    auto response = send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), endpoint("/channels/%/invites", id));
    for (auto const &each : response) {
        return_vec.emplace_back(each);
    }
    return return_vec;
}

discord::Invite discord::Channel::create_invite(int max_age, int max_uses, bool temporary, bool unique) const {
    nlohmann::json data = nlohmann::json({ { "max_age", max_age },
                                           { "max_uses", max_uses },
                                           { "temporary", temporary },
                                           { "unique", unique } });
    return discord::Invite{ send_request<request_method::Post>(data, get_default_headers(), endpoint("/channels/%/invites", id)) };
}

std::vector<discord::Message> discord::Channel::get_pins() {
    std::vector<discord::Message> message_vec;
    auto reply = send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), endpoint("/channels/%/pins", id));
    for (auto const &each : reply) {
        message_vec.emplace_back(each);
    }
    return message_vec;
}

discord::Webhook discord::Channel::create_webhook(std::string const &name) {
    return discord::Webhook{
        send_request<request_method::Post>(
            nlohmann::json({ { "name", name } }),
            get_default_headers(),
            endpoint("/channels/%/webhooks", id))
    };
}

std::vector<discord::Webhook> discord::Channel::get_webhooks() {
    return from_json_array<discord::Webhook>(
        send_request<request_method::Get>(nlohmann::json(),
                                          get_default_headers(),
                                          endpoint("/channels/%/webhooks", id)));
}

void discord::Channel::remove_permissions(discord::Object const &obj) {
    send_request<request_method::Delete>(nlohmann::json({}), get_default_headers(), format("%/channels/%/permissions/%", id, obj.id));
}

void discord::Channel::typing() {
    send_request<request_method::Post>(nlohmann::json({}), get_default_headers(), endpoint("/channels/%/typing", id));
}

void discord::Channel::add_group_dm_recipient(discord::User const &user, std::string const &access_token, std::string const &nick) {
    send_request<request_method::Put>(
        nlohmann::json({ { "access_token", access_token }, { "nick", nick } }),
        get_default_headers(),
        endpoint("/channels/%/recipient/%", this->id, user.id));
}

void discord::Channel::remove_group_dm_recipient(discord::User const &user) {
    send_request<request_method::Delete>(nlohmann::json({}), get_default_headers(), endpoint("/channels/%/recipient/%", this->id, user.id));
}

void discord::Channel::edit_position(int new_pos) {
    send_request<request_method::Patch>(
        nlohmann::json({ { "id", this->id }, { "position", new_pos } }),
        get_default_headers(),
        endpoint("/guilds/%/channels", this->guild->id));
}
