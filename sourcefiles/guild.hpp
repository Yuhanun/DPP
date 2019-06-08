#pragma once
#include <nlohmann/json.hpp>
#include "discord.hpp"

#include "utility.hpp"

#include "audit.hpp"
#include "channel.hpp"
#include "emoji.hpp"
#include "member.hpp"
#include "role.hpp"

discord::Guild::Guild(snowflake id)
    : discord::Object(id) {
    auto g = discord::utils::get(discord::detail::bot_instance->guilds, [id](auto const& guild) {
        return guild->id == id;
    });
    if (g) {
        *this = *g;
    }
}

discord::Guild::Guild(nlohmann::json const guild)
    : discord::Object(to_sf(guild["id"])),
      mfa_level{ get_value(guild, "mfa_level", 0) },
      afk_timeout{ get_value(guild, "afk_timeout", 0) },
      member_count{ get_value(guild, "member_count", 0) },
      verification_level{ get_value(guild, "verification_level", 0) },
      explicit_content_filter{ get_value(guild, "explicit_content_filter", 0) },
      large{ get_value(guild, "large", true) },
      unavailable{ get_value(guild, "unavailable", false) },
      application_id{ to_sf(get_value(guild, "application_id", "0")) },
      name{ get_value(guild, "name", "") },
      region{ get_value(guild, "region", "") },
      created_at{ time_from_discord_string(get_value(guild, "joined_at", "")) },
      vanity_url_code{ get_value(guild, "vanity_url_code", "") },
      emojis{ from_json_array<discord::Emoji>(guild, "emojis") } {
    auto g = discord::utils::get(discord::detail::bot_instance->guilds, [this](auto const& g) { return this->id == g->id; });

    for (auto const& each : guild["roles"]) {
        roles.push_back(std::make_shared<discord::Role>(each, g));
    }

    if (guild.contains("members")) {
        for (auto& each : guild["members"]) {
            auto member = std::make_shared<discord::Member>(each, g);
            members.push_back(member);
            if (each["user"]["id"] == guild["owner_id"]) {
                owner = member;
            }
            for (auto& pres : guild["presences"]) {
                if (to_sf(pres["user"]["id"]) == member->id) {
                    member->presence = discord::Presence{ pres };
                }
            }
        }
    }

    if (guild.contains("splash")) {
        if (!guild["splash"].is_null()) {
            std::string splash_hash = guild["splash"];
            icon = Asset{
                splash_hash, guild_splash, splash_hash[0] == 'a' && splash_hash[1] == '_', id
            };
        } else {
            icon = Asset{};
        }
    }

    if (guild.contains("icon")) {
        if (!guild["icon"].is_null()) {
            std::string av_hash = guild["icon"];
            icon = Asset{
                av_hash, guild_icon, av_hash[0] == 'a' && av_hash[1] == '_', id
            };
        } else {
            icon = Asset{};
        }
    }

    if (guild.contains("banner")) {
        if (!guild["banner"].is_null()) {
            std::string av_hash = guild["banner"];
            icon = Asset{
                av_hash, guild_banner, av_hash[0] == 'a' && av_hash[1] == '_', id
            };
        } else {
            icon = Asset{};
        }
    }

    if (guild.contains("channels") && channels.empty()) {
        for (auto const& each : guild["channels"]) {
            channels.emplace_back(std::make_shared<discord::Channel>(each, id));
        }
    }
}

discord::Guild& discord::Guild::update(nlohmann::json const data) {
    update_object_bulk(data,
                       "mfa_level", mfa_level,
                       "afk_timeout", afk_timeout,
                       "verification_level", verification_level,
                       "explicit_content_filter", explicit_content_filter,
                       "unavailable", unavailable,
                       "application_id", application_id,
                       "name", name,
                       "region", region,
                       "vanity_url_code", vanity_url_code);

    auto cur_guild = discord::utils::get(discord::detail::bot_instance->guilds, [this](auto const& g) { return this->id == g->id; });

    if (data.contains("roles")) {
        roles.clear();
        for (auto const& each : data["roles"]) {
            roles.emplace_back(std::make_shared<discord::Role>(each, cur_guild));
        }
    }

    if (data.contains("emojis")) {
        emojis = from_json_array<discord::Emoji>(data["emojis"]);
    }

    if (data.contains("splash")) {
        if (!data["splash"].is_null()) {
            std::string splash_hash = data["splash"];
            icon = Asset{
                splash_hash, guild_splash, splash_hash[0] == 'a' && splash_hash[1] == '_', id
            };
        } else {
            icon = Asset{};
        }
    }

    if (data.contains("icon")) {
        if (!data["icon"].is_null()) {
            std::string av_hash = data["icon"];
            icon = Asset{
                av_hash, guild_icon, av_hash[0] == 'a' && av_hash[1] == '_', id
            };
        } else {
            icon = Asset{};
        }
    }

    if (data.contains("banner")) {
        if (!data["banner"].is_null()) {
            std::string av_hash = data["banner"];
            icon = Asset{
                av_hash, guild_banner, av_hash[0] == 'a' && av_hash[1] == '_', id
            };
        } else {
            icon = Asset{};
        }
    }
    return *this;
}

pplx::task<std::vector<discord::Webhook>> discord::Guild::get_webhooks() {
    return send_request(methods::GET,
                        endpoint("/guilds/%/webhooks", id), id, guild)
        .then([](request_response const& response) {
            return from_json_array<discord::Webhook>(response.get().unwrap());
        });
}

pplx::task<void> discord::Guild::leave() {
    return send_request(methods::DEL,
                        endpoint("/users/@me/guilds/%", id),
                        id, guild)
        .then([](request_response const&) {});
}

pplx::task<std::vector<discord::Emoji>> discord::Guild::list_emojis() {
    return send_request(methods::GET,
                        endpoint("/guilds/%/emojis", id),
                        id, guild)
        .then([](request_response const& val) { return from_json_array<discord::Emoji>(val.get().unwrap()); });
}

pplx::task<discord::Emoji> discord::Guild::get_emoji(discord::Emoji const& e) {
    return send_request(methods::GET,
                        endpoint("/guilds/%/emojis/%", id, e.id), id, guild)
        .then([=](request_response const& resp) {
            return discord::Emoji{ resp.get().unwrap() };
        });
}

pplx::task<discord::Emoji> discord::Guild::edit_emoji(discord::Emoji const& emote, std::string name, std::vector<discord::Role> roles) {
    nlohmann::json data(
        { { "name", name },
          { "roles", {} } });

    for (auto const& each : roles) {
        data["roles"].push_back(each.id);
    }
    return send_request(methods::PATCH,
                        endpoint("/guilds/%/emojis/%", id, emote.id),
                        id, guild,
                        data)
        .then([=](request_response const& resp) { return discord::Emoji{ resp.get().unwrap() }; });
}

pplx::task<void> discord::Guild::edit(std::string const& name, std::string const& rg, int verif_level, int default_message_notif, int explicit_cont_filt, snowflake afk_chan_id, int afk_timeout, std::string const& icon, snowflake owner_id, std::string const& splash, snowflake system_channel_id) {
    nlohmann::json data{
        { "name", name }
    };
    if (rg != "") data["region"] = rg;
    if (verif_level != -1) data["verification_level"] = verif_level;
    if (default_message_notif != -1) data["default_message_notifications"] = default_message_notif;
    if (explicit_cont_filt != -1) data["explicit_content_filter"] = explicit_cont_filt;
    if (afk_chan_id != -1) data["afk_channel_id"] = afk_chan_id;
    if (afk_timeout != -1) data["afk_timeout"] = afk_timeout;
    if (icon != "") data["icon"] = icon;
    if (owner_id != -1) data["owner_id"] = owner_id;
    if (splash != "") data["splash"] = splash;
    if (system_channel_id != -1) data["system_channel_id"] = system_channel_id;

    return send_request(methods::PATCH,
                        endpoint("/guilds/%", id),
                        id, guild,
                        data)
        .then([](request_response const&) {});
}

pplx::task<void> discord::Guild::remove() {
    return send_request(methods::DEL,
                        endpoint("/guilds/%", id),
                        id, guild)
        .then([](request_response const&) {});
}

pplx::task<std::vector<discord::Channel>> discord::Guild::get_channels() {
    return send_request(methods::GET, endpoint("/guilds/%/channels", id), id, guild).then([=](request_response const& resp) {
        return from_json_array_special<discord::Channel>(resp.get().unwrap(), id);
    });
}

pplx::task<discord::Channel> discord::Guild::create_channel(std::string const& name, bool nsfw, int type, std::string const& topic, int bitrate, int user_limit, int rate_limit_per_user, int position, std::vector<discord::PermissionOverwrites> const& permission_overwrites, snowflake parent_id) {
    nlohmann::json data = nlohmann::json({ { "name", name },
                                           { "nsfw", nsfw },
                                           { "permission_overwrites", nlohmann::json::array() } });
    if (type != -1) data["type"] = type;
    if (topic != "") data["topic"] = topic;
    if (bitrate != -1) data["bitrate"] = bitrate;
    if (user_limit != -1) data["user_limit"] = user_limit;
    if (rate_limit_per_user != -1) data["rate_limit_per_user"];
    if (position != -1) data["position"] = position;

    for (auto const& each : permission_overwrites) {
        data["permission_overwrites"].push_back(each.to_json());
    }

    if (parent_id != -1) data["parent_id"] = parent_id;

    return send_request(methods::POST, endpoint("/guilds/%/channels", id), id, guild, data)
        .then([](request_response const& resp) {
            return discord::Channel{ resp.get().unwrap() };
        });
}


pplx::task<void> discord::Guild::remove_emoji(discord::Emoji const& emote) {
    return send_request(methods::DEL,
                        endpoint("/guilds/%/emojis/%", id, emote.id),
                        id, guild)
        .then([](request_response const&) {});
}

pplx::task<discord::Member> discord::Guild::get_member(snowflake m_id) {
    return send_request(methods::GET,
                        endpoint("/guilds/%/members/%", id, m_id), id, guild)
        .then([=](request_response const& resp) {
            return discord::Member{
                resp.get().unwrap(),
                discord::utils::get(discord::detail::bot_instance->guilds, [this](auto const& g) { return id == g->id; })
            };
        });
}

pplx::task<std::vector<discord::Member>> discord::Guild::get_members(int limit, snowflake after) {
    return send_request(methods::GET,
                        endpoint("/guilds/%/members", this->id), id, guild,
                        { { "limit", limit }, { "after", after } })
        .then([](request_response const& resp) {
            return from_json_array<discord::Member>(resp.get().unwrap());
        });
}

pplx::task<std::vector<std::pair<std::string, discord::User>>> discord::Guild::get_bans() {
    return send_request(methods::GET,
                        endpoint("/guilds/%/bans", id),
                        id, guild)
        .then([=](request_response const& resp) {
            std::vector<std::pair<std::string, discord::User>> ret_vec;
            for (auto const& each : resp.get().unwrap()) {
                ret_vec.push_back({ each["reason"], discord::User{ each["user"] } });
            }
            return ret_vec;
        });
}

pplx::task<std::pair<std::string, discord::User>> discord::Guild::get_ban(discord::Object const& banned_obj) {
    return send_request(methods::GET,
                        endpoint("/guilds/%/bans/%", id, banned_obj.id),
                        id, guild)
        .then([](request_response const& response) {
            return std::make_pair(response.get().unwrap()["reason"].get<std::string>(), discord::User{ response.get().unwrap()["user"] });
        });
}

pplx::task<void> discord::Guild::add_member(nlohmann::json const& data, snowflake user_id) {
    return send_request(methods::PUT,
                        endpoint("/guilds/%/members/%", this->id, user_id),
                        id, guild, data)
        .then([](auto const&) {});
}

pplx::task<void> discord::Guild::edit_bot_username(std::string const& new_nick) {
    return send_request(methods::PATCH,
                        endpoint("/guilds/%/members/@me/nick", id), id, guild,
                        { { "nick", new_nick } })
        .then([](request_response const&) {});
}

pplx::task<void> discord::Guild::unban(discord::Object const& obj) {
    return send_request(
               methods::DEL,
               endpoint("/guilds/%/bans/%", id, obj.id), id, guild)
        .then([](request_response const&) {});
}

pplx::task<std::vector<discord::Role>> discord::Guild::get_roles() {
    return send_request(
               methods::GET, endpoint("/guilds/%/roles", id),
               id, guild)
        .then([this](request_response const& resp) {
            return from_json_array_special<discord::Role>(resp.get().unwrap(),
                                                          discord::utils::get(discord::detail::bot_instance->guilds,
                                                                              [this](auto const& gld) {
                                                                                  return gld->id == this->id;
                                                                              }));
        });
}

pplx::task<discord::Role> discord::Guild::create_role(std::string const& _name, PermissionOverwrites& _perms, discord::Color _color, bool _hoist, bool _mention) {
    return send_request(methods::POST,
                        endpoint("/guilds/%/roles", id),
                        id, guild,
                        { { "name", _name },
                          { "permissions", _perms.base_permissions },
                          { "color", _color.raw_int },
                          { "hoist", _hoist },
                          { "mentionable", _mention } })
        .then([=](request_response const& val) {
            return discord::Role{
                val.get().unwrap(),
                discord::utils::get(discord::detail::bot_instance->guilds, [=](auto const& g) { return g->id == this->id; })
            };
        });
}

pplx::task<int> discord::Guild::get_prune_count(int days) {
    return send_request(methods::GET,
                        endpoint("/guilds/%/prune", id), id, guild,
                        { { "days", days } })
        .then([](request_response const& resp) { return resp.get().unwrap()["pruned"].get<int>(); });
}

pplx::task<int> discord::Guild::begin_prune(int days, bool compute_prune_count) {
    return send_request(methods::POST,
                        endpoint("/guilds/%/prune", id), id, guild,
                        { { "days", days },
                          { "compute_prune_count", compute_prune_count } })
        .then([](request_response const& resp) { return get_value(resp.get().unwrap(), "pruned", 0); });
}

pplx::task<std::vector<discord::VoiceRegion>> discord::Guild::get_voice_regions() {
    return send_request(methods::GET, endpoint("/guilds/%/regions", id), id, guild)
        .then([=](request_response const& resp) {
            std::vector<VoiceRegion> ret_val;
            for (auto const& each : resp.get().unwrap()) {
                ret_val.push_back({ each["id"],
                                    each["name"],
                                    each["vip"],
                                    each["optimal"],
                                    each["deprecated"],
                                    each["custom"] });
            }
            return ret_val;
        });
}

pplx::task<std::vector<discord::Invite>> discord::Guild::get_invites() {
    return send_request(methods::GET,
                        endpoint("/guilds/%/invites", id), id, guild)
        .then([=](request_response const& resp) {
            return from_json_array<discord::Invite>(resp.get().unwrap());
        });
}

pplx::task<discord::snowflake> discord::Guild::get_embed() {
    return send_request(methods::GET,
                        endpoint("/guilds/%/embed", id), id, guild)
        .then([](request_response const& resp) {
            return get_value<snowflake>(resp.get().unwrap(), "channel_id", 0);
        });
}

pplx::task<discord::snowflake> discord::Guild::edit_embed(snowflake c_id) {
    return send_request(methods::PATCH,
                        endpoint("/guilds/%/embed", id), id, guild,
                        { { "enabled", c_id != -1 }, { "channel_id", c_id } })
        .then([](request_response const& resp) {
            return get_value(resp.get().unwrap(), "channel_id", snowflake(0));
        });
}

pplx::task<std::string> discord::Guild::get_vanity_invite_url() {
    return send_request(methods::GET,
                        endpoint("/guilds/%/vanity-url", id), id, guild)
        .then([](request_response const& resp) {
            return get_value(resp.get().unwrap(), "code", "");
        });
}


discord::Asset discord::Guild::get_widget_image(std::string const& style) {
    discord::Asset a{};
    a.url = endpoint("/guilds/%/widget.png?style=%", id, style);
    return a;
}


pplx::task<std::vector<discord::Integration>> discord::Guild::get_integrations() {
    return send_request(methods::GET,
                        endpoint("/guilds/%/integrations", id), id, guild)
        .then([](request_response const& resp) {
            return from_json_array<discord::Integration>(resp.get().unwrap());
        });
}


pplx::task<void> discord::Guild::create_integration(discord::Integration const& integr) {
    return send_request(methods::POST,
                        endpoint("/guilds/%/integrations", id), id, guild,
                        { { "type", integr.type }, { "id", integr.id } })
        .then([](request_response const&) {});
}

pplx::task<void> discord::Guild::edit_integration(discord::Integration const& integr, int expire_behavior, int expire_grace_period, bool enable_emotes) {
    return send_request(methods::PATCH,
                        endpoint("/guilds/%/integrations/%", id, integr.id), id, guild,
                        { { "expire_behavior", expire_behavior },
                          { "expire_grace_period", expire_grace_period },
                          { "enable_emoticons", enable_emotes } })
        .then([](request_response const&) {});
}


pplx::task<void> discord::Guild::remove_integration(discord::Integration const& integr) {
    return send_request(methods::DEL,
                        endpoint("/guilds/%/integrations/%", id, integr.id),
                        id, guild)
        .then([](request_response const&) {});
}

pplx::task<void> discord::Guild::sync_integration(discord::Integration const& integr) {
    return send_request(methods::POST,
                        endpoint("/guilds/%/integrations/%/sync", id, integr.id),
                        id, guild)
        .then([](request_response const&) {});
}

pplx::task<discord::Emoji> discord::Guild::create_emoji(std::string const& name, discord::Emoji& emote_data, std::vector<discord::Role> roles) {
    nlohmann::json data({ { "name", name },
                          { "image", encode64(emote_data.image.read()) },
                          { "roles", nlohmann::json::array() } });
    for (auto const& each : roles) {
        data["roles"].push_back(each.id);
    }
    return send_request(methods::POST,
                        endpoint("/guilds/%/emojis", id),
                        id, guild,
                        data)
        .then([](request_response const& data) { return discord::Emoji{ data.get().unwrap() }; });
}

pplx::task<discord::AuditLogs> discord::Guild::get_audit_logs() {
    return send_request(methods::GET,
                        endpoint("/guilds/%/audit-logs", id),
                        id, guild)
        .then([](request_response const& resp) {
            return AuditLogs{ resp.get().unwrap() };
        });
}