#pragma once
#include <discord.hpp>
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
            nlohmann::json(),
            get_default_headers(),
            format("%/webhooks/%", get_api(), id))
    };
}

discord::Webhook::Webhook(snowflake id, std::string const& token) {
    *this = discord::Webhook{
        send_request<request_method::Get>(
            nlohmann::json(),
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
            nlohmann::json({{"name", name}}),
            get_default_headers(),
            get_edit_webhook_url())
    };
}

std::string discord::Webhook::get_edit_webhook_url() {
    return format("%/webhooks/%", get_api(), id);
}

std::string discord::Webhook::get_edit_webhook_token_url() {
    return format("%/webhooks/%/%", get_api(), id, token);
}