#pragma once
#include <discord.hpp>
#include <embedbuilder.hpp>
#include <string>
#include <utility.hpp>

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

discord::Webhook::Webhook(snowflake id) {
    *this = discord::Webhook{
        send_request<request_method::Get>(
            nlohmann::json({}),
            get_default_headers(),
            endpoint("/webhooks/%", id))
    };
}

discord::Webhook::Webhook(snowflake id, std::string const& token) {
    *this = discord::Webhook{
        send_request<request_method::Get>(
            nlohmann::json({}),
            get_default_headers(),
            endpoint("/webhooks/%/%", id, token))
    };
}

void discord::Webhook::edit(std::string const& name, snowflake chann_id) {
    nlohmann::json data{};
    if (!name.empty()) {
        data["name"] = name;
    }
    if (id) {
        data["channel_id"] = chann_id;
    }

    *this = discord::Webhook{
        send_request<request_method::Patch>(
            data,
            get_default_headers(),
            endpoint("/webhooks/%", id))
    };
}

void discord::Webhook::edit(std::string const& name) {
    *this = discord::Webhook{
        send_request<request_method::Patch>(
            nlohmann::json({ { "name", name } }),
            get_default_headers(),
            endpoint("/webhooks/%", id))
    };
}

void discord::Webhook::remove() {
    send_request<request_method::Delete>(
        nlohmann::json({}),
        get_default_headers(),
        endpoint("/webhooks/%/%", id, token));
}

discord::Message discord::Webhook::send(std::string const& content, bool tts, std::string const& avatar_url, std::string const& username) {
    nlohmann::json j = nlohmann::json({ { "content", content }, { "embed", nlohmann::json::array() }, { "tts", tts } });

    if (!avatar_url.empty()) {
        j["avatar_url"] = avatar_url;
    }
    if (!username.empty()) {
        j["username"] = username;
    }

    return discord::Message{ send_request<request_method::Post>(j, get_default_headers(), endpoint("/webhooks/%/%?wait=true", id, token)) };
}

discord::Message discord::Webhook::send(std::vector<EmbedBuilder> const& embed_arr, bool tts, std::string const& content, std::string const& avatar_url, std::string const& username) {
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


    return discord::Message{ send_request<request_method::Post>(j, get_default_headers(), endpoint("/webhooks/%/%?wait=true", id, token)) };
}


void discord::Webhook::execute_slack(bool wait, nlohmann::json const data) {
    send_request<request_method::Post>(
        data,
        get_default_headers(),
        endpoint("/webhooks/%/%/slack?wait=%", id, token, wait));
}

void discord::Webhook::execute_github(bool wait, nlohmann::json const data) {
    send_request<request_method::Post>(
        data,
        get_default_headers(),
        endpoint("/webhooks/%/%/github?wait=%", id, token, wait));
}
