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
      avatar{ get_value(data, "avatar", "") },
      token{ get_value(data, "token", "") } {
    if (!data.contains("user")) {
        user = std::nullopt;
    } else {
        user = discord::User{ data["user"] };
    }
}

discord::Webhook::Webhook(snowflake id) {
    *this = discord::Webhook{
        send_request<request_method::Get>(
            nlohmann::json({}),
            get_default_headers(),
            format("%/webhooks/%", get_api(), id))
    };
}

discord::Webhook::Webhook(snowflake id, std::string const& token) {
    *this = discord::Webhook{
        send_request<request_method::Get>(
            nlohmann::json({}),
            get_default_headers(),
            format("%/webhooks/%/%", get_api(), id, token))
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
            get_edit_webhook_url())
    };
}

void discord::Webhook::edit(std::string const& name) {
    *this = discord::Webhook{
        send_request<request_method::Patch>(
            nlohmann::json({ { "name", name } }),
            get_default_headers(),
            get_edit_webhook_url())
    };
}

void discord::Webhook::remove() {
    send_request<request_method::Delete>(
        nlohmann::json({}),
        get_default_headers(),
        get_delete_webhook_url());
}

discord::Message discord::Webhook::send(std::string const& content, bool tts, std::string const& avatar_url, std::string const& username) {
    nlohmann::json j = nlohmann::json({ { "content", content }, { "embed", nlohmann::json::array() }, { "tts", tts } });

    if (!avatar_url.empty()) {
        j["avatar_url"] = avatar_url;
    }
    if (!username.empty()) {
        j["username"] = username;
    }

    return discord::Message{ send_request<request_method::Post>(j, get_default_headers(), get_execute_webhook_url()) };
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


    return discord::Message{ send_request<request_method::Post>(j, get_default_headers(), get_execute_webhook_url()) };
}

std::string discord::Webhook::get_edit_webhook_url() const {
    return format("%/webhooks/%", get_api(), id);
}

std::string discord::Webhook::get_edit_webhook_token_url() const {
    return format("%/webhooks/%/%", get_api(), id, token);
}

std::string discord::Webhook::get_delete_webhook_url() const {
    return format("%/webhooks/%/%", id, token);
}

std::string discord::Webhook::get_execute_webhook_url() const {
    return format("%/webhooks/%/%?wait=true", get_api(), id, token);
}

void discord::Webhook::execute_slack(bool wait, nlohmann::json const data) {
    send_request<request_method::Post>(
        data,
        get_default_headers(),
        format("%/webhooks/%/%/slack", get_api(), id, token));
}
