#include "invite.hpp"
#include "utils.hpp"
#include "guild.hpp"
#include "role.hpp"
#include "bot.hpp"

discord::Invite::Invite(nlohmann::json const data) {
    code = data["code"];
    uses = get_value(data, "uses", 0);
    max_age = get_value(data, "max_age", 0);
    max_uses = get_value(data, "max_uses", 0);
    created_at = time_from_discord_string(get_value(data, "created_at", ""));

    snowflake temp_guild_id = to_sf(data["guild"]["id"]);
    auto guild = discord::utils::get(discord::detail::bot_instance->guilds, [temp_guild_id](auto const& g) {
        return g->id == temp_guild_id;
    });

    snowflake temp_channel_id = to_sf(data["channel"]["id"]);

    channel = discord::utils::get(guild->channels, [temp_channel_id](auto const& c) {
        return c->id == temp_channel_id;
    });
}

discord::Invite::Invite(std::string const& code)
    : code{ code } {
}


pplx::task<discord::Invite> discord::Invite::get_invite() {
    return send_request(methods::GET,
                        endpoint("/invites/%", code),
                        0, global,
                        { { "with_counts", true } })
        .then([](pplx::task<Result<nlohmann::json>> const& resp) {
            return discord::Invite{ resp.get().unwrap() };
        });
}

pplx::task<void> discord::Invite::remove() {
    return send_request(methods::DEL,
                        endpoint("/invites/%", code),
                        0, global)
        .then([](pplx::task<Result<nlohmann::json>> const&) {});
}