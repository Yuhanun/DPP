#include "role.hpp"
#include "utils.hpp"
#include "object.hpp"
#include "guild.hpp"

discord::Role::Role(snowflake id)
    : discord::Object(id) {
    discord::utils::get(discord::detail::bot_instance->guilds, [id, this](auto const& g) {
        for (auto const& role : g->roles) {
            if (role->id == id) {
                *this = *role;
                return true;
            }
        }
        return false;
    });
}

discord::Role::Role(nlohmann::json data, std::shared_ptr<discord::Guild> g)
    : discord::Object(to_sf(data["id"])) {
    hoist = data["hoist"];
    managed = data["managed"];
    mentionable = data["mentionable"];

    name = data["name"];
    color = discord::Color(data["color"].get<int>());
    permissions = PermissionOverwrites(data["permissions"].get<int>(), 0, id, object_type::role);
    guild = g;
}

discord::Role& discord::Role::update(nlohmann::json data) {
    update_object_bulk(data,
                       "hoist", hoist,
                       "mentionable", mentionable,
                       "name", name,
                       "color", color);

    if (data.contains("permissions")) {
        permissions = PermissionOverwrites(data["permissions"].get<int>(), 0, id, object_type::role);
    }

    return *this;
}

pplx::task<void> discord::Role::edit_position(int _new_position) {
    return send_request(methods::POST,
                        endpoint("/guilds/%/roles", guild->id),
                        guild->id, bucket_type::guild,
                        { { "id", id }, { "position", _new_position } })
        .then([](auto const&) {});
}

pplx::task<discord::Role> discord::Role::edit(std::string const& _name, PermissionOverwrites& _perms, discord::Color _color, bool _hoist, bool _mention) {
    return send_request(methods::PATCH,
                        endpoint("/guilds/%/roles/%", guild->id, id),
                        guild->id, bucket_type::guild,
                        { { "name", _name },
                          { "permissions", _perms.base_permissions },
                          { "color", _color.raw_int },
                          { "hoist", _hoist },
                          { "mentionable", _mention } })
        .then([=](request_response const& resp) {
            return discord::Role{
                resp.get().unwrap(), this->guild
            };
        });
}

pplx::task<void> discord::Role::remove() {
    return send_request(methods::DEL,
                        endpoint("/guilds/%/roles/%", guild->id, id),
                        guild->id, bucket_type::guild)
        .then([](auto const&) {});
}