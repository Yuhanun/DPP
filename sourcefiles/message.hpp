#pragma once
#include <boost/date_time.hpp>
#include <locale>
#include <nlohmann/json.hpp>
#include "channel.hpp"
#include "discord.hpp"
#include "member.hpp"

discord::Message::Message(snowflake id)
    : id{ id } {
}

discord::Message discord::Message::from_sent_message(nlohmann::json j) {
    auto m = Message{};
    m.token = discord::detail::bot_instance->token;
    snowflake sender_id = to_sf(get_value(j["author"], "id", "0"));
    m.pinned = get_value(j, "pinned", false);
    m.tts = get_value(j, "tts", false);
    m.timestamp = discord::time_from_discord_string(get_value(j, "timestamp", ""));
    m.mention_everyone = get_value(j, "mention_everyone", false);
    m.id = to_sf(get_value(j, "id", "0"));
    snowflake channel_id = to_sf(j["channel_id"]);

    for (auto const& mention : j["mention_roles"]) {
        m.mentioned_roles.push_back(discord::Role{ to_sf(mention) });
    }

    m.channel = discord::utils::get(discord::detail::bot_instance->channels, [&channel_id](auto const& c) {
        return c->id == channel_id;
    });

    if (m.channel->guild) {
        m.author = discord::utils::get(m.channel->guild->members, [&sender_id](auto const& mem) {
            return mem.id == sender_id;
        });
    }

    for (auto const& mention : j["mentions"]) {
        snowflake mention_id = to_sf(mention["id"]);
        if (m.channel->guild) {
            auto mem = discord::utils::get(m.channel->guild->members, [&mention_id](auto const& mem) {
                return mem.id == mention_id;
            });
            if (mem) {
                m.mentions.push_back(*mem);
            }
        } else {
            m.mentions.push_back(discord::Member{ mention });
        }
    }

    for (auto const& embed : j["embeds"]) {
        m.embeds.push_back(discord::EmbedBuilder{ embed });
    }

    m.edited_timestamp = time_from_discord_string(get_value(j, "edited_timestamp", ""));

    m.content = get_value(j, "content", "");
    m.type = get_value(j, "type", 0);
    return m;
}

std::string discord::Message::get_delete_url() {
    return format("%/%/messages/%", get_api(), channel->id, id);
}

void discord::Message::remove() {
    send_request<request_method::Delete>(nlohmann::json({}), get_default_headers(), get_delete_url());
}

std::string discord::Message::get_edit_url() {
    return format("%/channels/%/messages/%", get_api(), channel->id, id);
}

discord::Message discord::Message::edit(std::string content) {
    nlohmann::json j = nlohmann::json({ { "content", content }, { "tts", tts } });
    auto response = send_request<request_method::Patch>(j, get_default_headers(), get_edit_url());
    return discord::Message::from_sent_message(response);
}

discord::Message discord::Message::edit(EmbedBuilder embed, std::string content) {
    nlohmann::json j = nlohmann::json({ { "content", content }, { "tts", tts }, { "embed", embed.to_json() } });
    auto response = send_request<request_method::Patch>(j, get_default_headers(), get_edit_url());
    return discord::Message::from_sent_message(response);
}

std::string discord::Message::get_pin_url() {
    return format("%/channels/%/pins/%", get_api(), channel->id, id);
}

std::string discord::Message::get_unpin_url() {
    return format("%/channels/%/pins/%", get_api(), channel->id, id);
}

void discord::Message::unpin() {
    send_request<request_method::Delete>(nlohmann::json({}), get_default_headers(), get_pin_url());
}

void discord::Message::pin() {
    send_request<request_method::Put>(nlohmann::json({}), get_default_headers(), get_pin_url());
}