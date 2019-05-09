#pragma once
#include <discord.hpp>
#include <invite.hpp>
#include "permissions.hpp"
#include "utility.hpp"

discord::Channel::Channel(snowflake id) {
    this->id = id;
    for (auto const &guild : discord::detail::bot_instance->guilds) {
        for (auto const &channel : guild->channels) {
            if (channel.id == id) {
                *this = channel;
            }
        }
    }

    for (auto const &channel : discord::detail::bot_instance->channels) {
        if (channel->id == id){
            *this = *(channel);
        }
    }
}

discord::Channel::Channel(nlohmann::json const data, snowflake guild_id) {
    type = data["type"];
    if (data.contains("bitrate")) {
        if (data.contains("parent_id")) {
            parent_id = to_sf(get_value(data, "parent_id", "0"));
        }
        bitrate = data["bitrate"];
        user_limit = data["user_limit"];
    } else {
        parent_id = to_sf(get_value(data, "parent_id", "0"));
        rate_limit_per_user = get_value(data, "rate_limit_per_user", 0);
        topic = discord::get_value(data, "topic", "");
    }

    guild = nullptr;
    if (guild_id) {
        for (auto const &v_guild : discord::detail::bot_instance->guilds) {
            if (v_guild->id != guild_id) {
                continue;
            }
            guild = v_guild.get();
            break;
        }
    }

    if (type == channel_type::dm_channel || type == channel_type::group_dm_channel) {
        for (auto const &each : data["recipients"]) {
            recipients.push_back(discord::User{ each });
        }
    } 

    if (data.contains("permission_overwrites")) {
        for (auto &each : data["permission_overwrites"]) {
            int t = each["type"].get<std::string>() == "role" ? role : member;
            overwrites.push_back(discord::PermissionOverwrites{
                each["allow"].get<int>(), each["deny"].get<int>(),
                to_sf(each["id"]), t });
        }
    }
    if (data.contains("name")) {
        name = data["name"];
    }
    if (data.contains("position")){
        position = data["position"];
    }
    id = to_sf(data["id"]);
}

discord::Message discord::Channel::send(std::string content, bool tts) const {
    return discord::detail::bot_instance->send_message(id, content, tts);
}

discord::Message discord::Channel::send(EmbedBuilder embed, bool tts, std::string content) const {
    nlohmann::json j = nlohmann::json({ { "embed", embed.to_json() }, { "tts", tts } });

    if (content != "") {
        j["content"] = content;
    }

    return discord::detail::bot_instance->send_message(id, j, tts);
}

std::string discord::Channel::get_bulk_delete_url() {
    return format("%/channels/%/messages/bulk-delete", get_api(), id);
}

std::string discord::Channel::get_get_messages_url(int limit) {
    return format("%/channels/%/messages?limit=%", get_api(), id, limit);
}

void discord::Channel::bulk_delete(std::vector<discord::Message> &m) {
    nlohmann::json array = nlohmann::json::array();
    for (auto const &each : m) {
        array.push_back(each.id);
    }
    nlohmann::json data = nlohmann::json();
    data["messages"] = array;
    send_request<request_method::Post>(data, get_default_headers(), get_bulk_delete_url());
}

std::vector<discord::Message> discord::Channel::get_messages(int limit) {
    std::vector<discord::Message> return_vec;
    limit = limit < 1 || limit > 100 ? 50 : limit;

    auto data = send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), get_get_messages_url(limit));
    for (auto &each : data) {
        return_vec.push_back(discord::Message::from_sent_message(each));
    }
    return return_vec;
}

std::string discord::Channel::get_channel_edit_url() {
    return format("%/channels/%", get_api(), id);
}

std::string discord::Channel::get_delete_url() {
    return format("%/channels/%", get_api(), id);
}

void discord::Channel::edit(nlohmann::json &j) {
    send_request<request_method::Patch>(j, get_default_headers(), get_channel_edit_url());
}

void discord::Channel::remove() {
    send_request<request_method::Delete>(nlohmann::json({}), get_default_headers(), get_delete_url());
}

discord::Message discord::Channel::get_message(snowflake id) {
    return discord::Message::from_sent_message(
        send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), get_get_message_url(id)));
}

std::vector<discord::Invite> discord::Channel::get_invites() {
    std::vector<discord::Invite> return_vec;
    auto response = send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), get_channel_invites_url());
    for (auto const &each : response) {
        return_vec.push_back(discord::Invite{ each });
    }
    return return_vec;
}

discord::Invite discord::Channel::create_invite(int max_age, int max_uses, bool temporary, bool unique) const {
    nlohmann::json data = nlohmann::json({ { "max_age", max_age },
                                           { "max_uses", max_uses },
                                           { "temporary", temporary },
                                           { "unique", unique } });
    return discord::Invite{ send_request<request_method::Post>(data, get_default_headers(), get_create_invite_url()) };
}

std::vector<discord::Message> discord::Channel::get_pins() {
    std::vector<discord::Message> message_vec;
    auto reply = send_request<request_method::Get>(nlohmann::json({}), get_default_headers(), get_pins_url());
    for (auto const &each : reply) {
        message_vec.push_back(discord::Message::from_sent_message(each));
    }
    return message_vec;
}

void discord::Channel::remove_permissions(discord::Object const &obj) {
    send_request<request_method::Delete>(nlohmann::json({}), get_default_headers(), get_delete_channel_permission_url(obj));
}

void discord::Channel::typing() {
    send_request<request_method::Post>(nlohmann::json({}), get_default_headers(), get_typing_url());
}

std::string discord::Channel::get_get_message_url(snowflake m_id) {
    return format("%/channels/%/messages/%", get_api(), id, m_id);
}

std::string discord::Channel::get_channel_invites_url() {
    return format("%/channels/%/invites", get_api(), id);
}

std::string discord::Channel::get_create_invite_url() const {
    return format("%/channels/%/invites", get_api(), id);
}

std::string discord::Channel::get_delete_channel_permission_url(
    discord::Object const &obj) {
    return format("%/channels/%/permissions/%", id, obj.id);
}

std::string discord::Channel::get_typing_url() {
    return format("%/channels/%/typing", get_api(), id);
}

std::string discord::Channel::get_pins_url() {
    return format("%/channels/%/pins", get_api(), id);
}