#pragma once
#include <discord.hpp>
#include <nlohmann/json.hpp>
#include "channel.hpp"
#include "member.hpp"

discord::Message::Message(snowflake id)
    : id{ id } {
}

discord::Message discord::Message::from_sent_message(std::string data) {
    auto j = json::parse(data);
    auto m = Message{};
    m.token = discord::bot_instance->token;
    snowflake sender_id = std::stoul(j["author"]["id"].get<std::string>());
    m.sent = true;
    m.tts = j["tts"];
    m.timestamp = j["timestamp"];
    m.mention_everyone = j["mention_everyone"];
    m.id = std::stoul(j["id"].get<std::string>());
    snowflake channel_id = std::stoul(j["channel_id"].get<std::string>());

    for (auto const &chan : discord::bot_instance->channels) {
        if (chan->id == channel_id) {
            m.channel = *(chan.get());
            break;
        }
    }

    for (auto const &member : m.channel.guild->members) {
        if (member.id != sender_id) {
            continue;
        }
        m.author = member;
        break;
    }

    m.content = j["content"];
    m.type = j["type"];
    return m;
}

std::string discord::Message::get_delete_url() {
    return std::string("https://discordapp.com/api/v6/channels/") +
           std::to_string(channel.id) + std::string("/messages/") +
           std::to_string(id);
}

void discord::Message::remove() {
    send_request<request_method::Delete>(json({}), get_default_headers(), get_delete_url());
}

std::string discord::Message::get_edit_url() {
    return format("%/channels/%/messages/%", get_api(), channel.id, id);
}

discord::Message discord::Message::edit(std::string content) {
    json j = json({ { "content", content }, { "tts", tts } });
    auto response = send_request<request_method::Patch>(j, get_default_headers(), get_edit_url());
    return discord::Message::from_sent_message(response.get().dump());
}

discord::Message discord::Message::edit(EmbedBuilder embed,
                                        std::string content) {
    json j =
        json({ { "content", content }, { "tts", tts }, { "embed", embed.to_json() } });

    auto response =
        send_request<request_method::Patch>(j, get_default_headers(), get_edit_url());
    return discord::Message::from_sent_message(response.get().dump());
}

std::string discord::Message::get_pin_url() {
    return format("%/channels/%/pins/%", get_api(), channel.id, id);
}

std::string discord::Message::get_unpin_url() {
    return format("%/channels/%/pins/%", get_api(), channel.id, id);
}

void discord::Message::unpin() {
    send_request<request_method::Delete>(json({}), get_default_headers(), get_pin_url());
}

void discord::Message::pin() {
    send_request<request_method::Put>(json({}), get_default_headers(), get_pin_url());
}