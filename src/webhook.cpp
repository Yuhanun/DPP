#include <string>
#include "utils.hpp"
#include "webhook.hpp"
#include "user.hpp"
#include "message.hpp"
#include "emoji.hpp"
#include "embedbuilder.hpp"
#include "role.hpp"
#include "attachment.hpp"

discord::Webhook::Webhook(nlohmann::json const data)
    : id{ to_sf(get_value(data, "id", "0")) },
      guild{ to_sf(get_value(data, "guild_id", "0")) },
      channel{ to_sf(get_value(data, "channel_id", "0")) },
      name{ get_value(data, "name", "") },
      token{ get_value(data, "token", "") } {
    if (!data.contains("user")) {
        user = std::nullopt;
    } else {
        user = discord::User{ data["user"] };
    }

    if (data.contains("avatar")) {
        if (data["avatar"].is_null()) {
            avatar = { "", default_user_avatar, false, static_cast<snowflake>(0) };
        } else {
            std::string av_hash = data["avatar"];
            avatar = { av_hash, user_avatar, av_hash[0] == 'a' && av_hash[1] == '_', id };
        }
    }
}

discord::Webhook::Webhook(snowflake id)
    : id{ id } {
}

discord::Webhook::Webhook(snowflake id, std::string const& token)
    : id{ id }, token{ token } {
}

pplx::task<discord::Webhook> discord::Webhook::edit(std::string const& name, snowflake chann_id) {
    nlohmann::json data{};
    if (!name.empty()) {
        data["name"] = name;
    }
    if (id) {
        data["channel_id"] = chann_id;
    }

    return send_request(methods::PATCH,
                        endpoint("/webhooks/%", id),
                        id, webhook,
                        data)
        .then([](request_response const& resp) {
            return discord::Webhook{ resp.get().unwrap() };
        });
}

pplx::task<discord::Webhook> discord::Webhook::edit(std::string const& name) {
    return send_request(methods::PATCH,
                        endpoint("/webhooks/%", id),
                        id, webhook,
                        { { "name", name } })
        .then([](request_response const& resp) {
            return discord::Webhook{ resp.get().unwrap() };
        });
}

pplx::task<void> discord::Webhook::remove() {
    return send_request(methods::DEL,
                        endpoint("/webhooks/%/%", id, token),
                        id, webhook)
        .then([](request_response const&) {});
}

pplx::task<discord::Message> discord::Webhook::send(std::string const& content, bool tts, std::string const& avatar_url, std::string const& username) {
    nlohmann::json j = nlohmann::json({ { "content", content }, { "embed", nlohmann::json::array() }, { "tts", tts } });

    if (!avatar_url.empty()) {
        j["avatar_url"] = avatar_url;
    }
    if (!username.empty()) {
        j["username"] = username;
    }

    return send_request(methods::POST,
                        endpoint("/webhooks/%/%?wait=true", id, token),
                        id, webhook,
                        j)
        .then([](request_response const& resp) {
            return discord::Message{ resp.get().unwrap() };
        });
}

pplx::task<discord::Message> discord::Webhook::send(std::vector<EmbedBuilder> const& embed_arr, bool tts, std::string const& content, std::string const& avatar_url, std::string const& username) {
    nlohmann::json j = nlohmann::json({ { "embed", nlohmann::json::array() }, { "tts", tts } });

    for (auto const& embed : embed_arr) {
        j["embed"].push_back(embed.to_json());
    }

    if (!content.empty()) {
        j["content"] = content;
    }
    if (!avatar_url.empty()) {
        j["avatar_url"] = avatar_url;
    }
    if (!username.empty()) {
        j["username"] = username;
    }


    return send_request(methods::POST,
                        endpoint("/webhooks/%/%?wait=true", id, token),
                        id, webhook, j)
        .then([](request_response const& resp) {
            return discord::Message{ resp.get().unwrap() };
        });
}


pplx::task<void> discord::Webhook::execute_slack(bool wait, nlohmann::json const data) {
    return send_request(methods::POST,
                        endpoint("/webhooks/%/%/slack?wait=%", id, token, wait),
                        id, webhook,
                        data)
        .then([](request_response const&) {});
}

pplx::task<void> discord::Webhook::execute_github(bool wait, nlohmann::json const data) {
    return send_request(methods::POST,
                        endpoint("/webhooks/%/%/github?wait=%", id, token, wait),
                        id, webhook,
                        data)
        .then([](request_response const&) {});
}
