#include <boost/date_time.hpp>
#include <locale>
#include <nlohmann/json.hpp>
#include "utils.hpp"
#include "message.hpp"
#include "channel.hpp"
#include "guild.hpp"
#include "attachment.hpp"
#include "embedbuilder.hpp"
#include "emoji.hpp"
#include "user.hpp"
#include "role.hpp"
#include "member.hpp"

discord::Message::Message(snowflake id)
    : discord::Object(id) {
}

discord::Message::Message(nlohmann::json const j)
    : discord::Object(to_sf(j["id"])) {
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

    channel = discord::utils::get(discord::detail::bot_instance->channels, [channel_id](auto const& c) {
        return c->id == channel_id;
    });

    if (!channel) {
        auto channel = std::make_shared<discord::Channel>(channel_id, 0);
        discord::detail::bot_instance->channels.push_back(channel);
    }

    if (channel && channel->guild) {
        author = discord::utils::get(channel->guild->members, [sender_id](auto const& mem) {
            return mem->id == sender_id;
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
            if (channel && channel->guild) {
                auto mem = discord::utils::get(channel->guild->members, [mention_id](auto const& mem) {
                    return mem->id == mention_id;
                });

                if (mem) {
                    mentions.push_back(mem);
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

discord::Message& discord::Message::update(nlohmann::json const j) {
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

    channel = discord::utils::get(discord::detail::bot_instance->channels, [channel_id](auto const& c) {
        return c->id == channel_id;
    });

    if (channel->guild) {
        author = discord::utils::get(channel->guild->members, [sender_id](auto const& mem) {
            return mem->id == sender_id;
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
                auto mem = discord::utils::get(channel->guild->members, [mention_id](auto const& mem) {
                    return mem->id == mention_id;
                });
                if (mem) {
                    mentions.emplace_back(mem);
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

pplx::task<void> discord::Message::remove() {
    return send_request(methods::DEL,
                        endpoint("/channels/%/messages/%", channel->id, id),
                        channel->id, bucket_type::channel)
        .then([](request_response const&) {});
}

pplx::task<discord::Message> discord::Message::edit(std::string content) {
    return send_request(methods::PATCH,
                        endpoint("/channels/%/messages/%", channel->id, id),
                        channel->id, bucket_type::channel,
                        { { "content", content }, { "tts", tts } })
        .then([](request_response const& resp) {
            return discord::Message{ resp.get().unwrap() };
        });
}

pplx::task<discord::Message> discord::Message::edit(EmbedBuilder embed, std::string content) {
    return send_request(methods::PATCH,
                        endpoint("/channels/%/messages/%", channel->id, id),
                        channel->id, bucket_type::channel,
                        { { "content", content }, { "tts", tts }, { "embed", embed.to_json() } })
        .then([](request_response const& resp) {
            return discord::Message{ resp.get().unwrap() };
        });
}

pplx::task<void> discord::Message::unpin() {
    return send_request(methods::DEL,
                        endpoint("/channels/%/pins/%", channel->id, id),
                        channel->id, bucket_type::channel)
        .then([](request_response const&) {});
}

pplx::task<void> discord::Message::pin() {
    return send_request(methods::PUT,
                        endpoint("/channels/%/pins/%", channel->id, id),
                        channel->id, bucket_type::channel)
        .then([](request_response const&) {});
}

pplx::task<void> discord::Message::add_reaction(discord::Emoji const& emote) {
    return send_request(methods::PUT,
                        endpoint("/channels/%/messages/%/reactions/%:%/@me", channel->id, id, emote.name, emote.id),
                        channel->id, bucket_type::channel)
        .then([](request_response const&) {});
}

pplx::task<void> discord::Message::remove_own_reaction(discord::Emoji const& emote) {
    return send_request(methods::DEL,
                        endpoint("/channels/%/messages/%/reactions/%:%/@me", channel->id, id, emote.name, emote.id),
                        channel->id, bucket_type::channel)
        .then([](request_response const&) {});
}

pplx::task<void> discord::Message::remove_reaction(discord::User const& user, discord::Emoji const& emote) {
    return send_request(methods::DEL,
                        endpoint("/channels/%/messages/%/reactions/%:%/%", channel->id, id, emote.name, emote.id, user.id),
                        channel->id, bucket_type::channel)
        .then([](request_response const&) {});
}

pplx::task<std::vector<discord::User>> discord::Message::get_reactions(discord::Emoji const& emote, snowflake before, snowflake after, int limit) {
    auto data = nlohmann::json({ { "limit", limit > 0 ? limit : 25 } });

    if (after) {
        data["after"] = after;
    }
    if (before) {
        data["before"] = before;
    }

    return send_request(methods::GET,
                        endpoint("/channels/%/messages/%/reactions/%:%", channel->id, id, emote.name, emote.id),
                        channel->id, bucket_type::channel,
                        data)
        .then([](request_response const& resp) {
            return from_json_array<discord::User>(resp.get().unwrap());
        });
}

pplx::task<void> discord::Message::remove_all_reactions() {
    return send_request(methods::DEL,
                        endpoint("/channels/%/messages/%/reactions", channel->id, id),
                        channel->id, bucket_type::channel)
        .then([](request_response const&) {});
}
