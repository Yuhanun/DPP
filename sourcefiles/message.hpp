#pragma once
#include <boost/date_time.hpp>
#include <locale>
#include <nlohmann/json.hpp>
#include "attachment.hpp"
#include "channel.hpp"
#include "discord.hpp"
#include "member.hpp"

discord::Message::Message(snowflake id)
    : id{ id } {
}

discord::Message::Message(nlohmann::json const j) {
    token = discord::detail::bot_instance->token;
    snowflake sender_id = to_sf(get_value(get_value(j, "author", nlohmann::json({ { "id", "0" } })), "id", "0"));
    pinned = get_value(j, "pinned", false);
    tts = get_value(j, "tts", false);
    timestamp = time_from_discord_string(get_value(j, "timestamp", ""));
    mention_everyone = get_value(j, "mention_everyone", false);
    id = to_sf(get_value(j, "id", "0"));
    snowflake channel_id = to_sf(get_value(j, "channel_id", "0"));

    if (j.contains("mention_roles")) {
        for (auto const& mention : j["mention_roles"]) {
            mentioned_roles.emplace_back(to_sf(mention));
        }
    }

    channel = discord::utils::get(discord::detail::bot_instance->channels, [&channel_id](auto const& c) {
        return c->id == channel_id;
    });

    if (channel->guild) {
        author = discord::utils::get(channel->guild->members, [&sender_id](auto const& mem) {
            return mem.id == sender_id;
        });
    }

    if (j.contains("mention_roles")) {
        for (auto const& attach : j["attachments"]) {
            attachments.emplace_back(attach);
        }
    }

    if (j.contains("mentions")) {
        for (auto const& mention : j["mentions"]) {
            snowflake mention_id = to_sf(mention["id"]);
            if (channel->guild) {
                auto mem = discord::utils::get(channel->guild->members, [&mention_id](auto const& mem) {
                    return mem.id == mention_id;
                });
                if (mem) {
                    mentions.push_back(*mem);
                }
            } else {
                mentions.emplace_back(mention);
            }
        }
    }

    if (j.contains("embeds")) {
        for (auto const& embed : j["embeds"]) {
            embeds.emplace_back(embed);
        }
    }
    edited_timestamp = time_from_discord_string(get_value(j, "edited_timestamp", ""));

    content = get_value(j, "content", "");
    type = get_value(j, "type", 0);
}

discord::Message& discord::Message::update(nlohmann::json const j){
    snowflake sender_id = to_sf(get_value(get_value(j, "author", nlohmann::json({ { "id", "0" } })), "id", "0"));
    pinned = get_value(j, "pinned", false);
    tts = get_value(j, "tts", false);
    timestamp = time_from_discord_string(get_value(j, "timestamp", ""));
    mention_everyone = get_value(j, "mention_everyone", false);
    id = to_sf(get_value(j, "id", "0"));
    snowflake channel_id = to_sf(get_value(j, "channel_id", "0"));

    if (j.contains("mention_roles")) {
        mentioned_roles.clear();
        for (auto const& mention : j["mention_roles"]) {
            mentioned_roles.emplace_back(to_sf(mention));
        }
    }

    channel = discord::utils::get(discord::detail::bot_instance->channels, [&channel_id](auto const& c) {
        return c->id == channel_id;
    });

    if (channel->guild) {
        author = discord::utils::get(channel->guild->members, [&sender_id](auto const& mem) {
            return mem.id == sender_id;
        });
    }

    if (j.contains("attachments")) {
        attachments.clear();
        for (auto const& attach : j["attachments"]) {
            attachments.emplace_back(attach);
        }
    }

    if (j.contains("mentions")) {
        mentions.clear();
        for (auto const& mention : j["mentions"]) {
            snowflake mention_id = to_sf(mention["id"]);
            if (channel->guild) {
                auto mem = discord::utils::get(channel->guild->members, [&mention_id](auto const& mem) {
                    return mem.id == mention_id;
                });
                if (mem) {
                    mentions.push_back(*mem);
                }
            } else {
                mentions.emplace_back(mention);
            }
        }
    }

    if (j.contains("embeds")) {
        embeds.clear();
        for (auto const& embed : j["embeds"]) {
            embeds.emplace_back(embed);
        }
    }
    return *this;
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
    return discord::Message{ response };
}

discord::Message discord::Message::edit(EmbedBuilder embed, std::string content) {
    nlohmann::json j = nlohmann::json({ { "content", content }, { "tts", tts }, { "embed", embed.to_json() } });
    auto response = send_request<request_method::Patch>(j, get_default_headers(), get_edit_url());
    return discord::Message{ response };
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