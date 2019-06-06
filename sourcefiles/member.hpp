#pragma once
#include "discord.hpp"
#include "user.hpp"

discord::Member::Member(snowflake id) {
    user->id = id;
    for (auto const& guild : discord::detail::bot_instance->guilds) {
        for (auto const& mem : guild->members) {
            if (id == mem->id) {
                *this = *mem;
            }
        }
    }
}

discord::Member::Member(nlohmann::json const j, std::shared_ptr<discord::Guild> g) {
    deaf = get_value(j, "deaf", false);
    muted = get_value(j, "mute", false);
    nick = get_value(j, "nick", "");
    joined_at = time_from_discord_string(get_value(j, "joined_at", ""));
    guild = g;
    auto usr_id = to_sf(j["user"]["id"]);
    user = discord::utils::get(discord::detail::bot_instance->users, [=](auto& usr) { return usr->id == usr_id; });
    if (user.get() == nullptr) {
        user = std::make_shared<discord::User>(j["user"]);
        discord::detail::bot_instance->users.push_back(user);
    }
    id = user->id;

    if (!j.contains("roles")) {
        return;
    }
    for (auto const& role : j["roles"]) {
        // TODO: fix, g can be nullptr if this is initialization stage...
        roles.push_back(std::make_shared<discord::Role>(to_sf(role))); // retrieve from g->roles
    }
}

discord::Member& discord::Member::update(nlohmann::json const data) {
    update_object(data, "nick", nick);
    update_object(data, "deaf", deaf);
    update_object(data, "mute", muted);
    return *this;
}

void discord::Member::edit(std::string const& t_name, bool t_mute, bool t_deaf, std::vector<discord::Role> const& t_roles, snowflake channel_id) {
    nlohmann::json data({ { "name", t_name },
                          { "mute", t_mute },
                          { "deaf", t_deaf },
                          { "roles", nlohmann::json::array() } });

    for (auto const& each : t_roles) {
        data["roles"].push_back(each.id);
    }

    if (channel_id != -1) {
        if (channel_id == 0)
            data["channel_id"] = nullptr;
        else
            data["channel_id"] = channel_id;
    }

    send_request<request_method::Patch>(
        data, get_default_headers(), endpoint("/guilds/%/members/%", this->guild->id, this->id));
}


void discord::Member::add_role(discord::Role const& new_role) {
    send_request<request_method::Put>(
        nlohmann::json({}),
        get_default_headers(),
        endpoint("/guilds/%/members/%/roles/%", this->guild->id, this->id, new_role.id));
}

void discord::Member::remove_role(discord::Role const& new_role) {
    send_request<request_method::Delete>(
        nlohmann::json({}),
        get_default_headers(),
        endpoint("/guilds/%/members/%/roles/%", this->guild->id, this->id, new_role.id));
}

void discord::Member::kick() {
    send_request<request_method::Delete>(
        nlohmann::json({}),
        get_default_headers(),
        endpoint("/guilds/%/members/%", this->guild->id, this->id));
}

void discord::Member::ban(std::string const& _reason, int _days) {
    send_request<request_method::Put>(
        nlohmann::json({ { "reason", _reason }, { "delete-message-days", _days } }),
        get_default_headers(),
        endpoint("/guilds/%/members/%", this->guild->id, this->id));
}
