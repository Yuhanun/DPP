#include "permissions.hpp"
#include "experimental/vector"
#include "utils.hpp"

discord::PermissionOverwrite::PermissionOverwrite(int value, int allow_type)
    : value{ value }, allow_type{ allow_type } {
}

void discord::PermissionOverwrite::calculate_value() {
    value = 0;
    for (auto const& each : ows) {
        if (each.second == allow_type) {
            value |= discord::utils::permission_overwrites[each.first];
        }
    }
}

void discord::PermissionOverwrite::set_table() {
    ows.clear();
    for (auto const& each : discord::utils::permission_overwrites) {
        if ((value & each.second) == value) {
            ows[each.first] = allow_type;
        }
    }
}

bool discord::PermissionOverwrite::has_permission(std::string const& perm_name) {
    int val = utils::permission_overwrites[perm_name];
    return (value & val) == val;
}


discord::PermissionOverwrite& discord::PermissionOverwrite::add_permission(std::string const& name) {
    ows[name] = allow_type;
    calculate_value();
    return *this;
}

discord::PermissionOverwrites::PermissionOverwrites(int allow_value, int deny_value, snowflake object_id, int object_type)
    : object_type{ object_type }, object_id{ object_id } {
    allow_perms = PermissionOverwrite(allow_value, allow);
    deny_perms = PermissionOverwrite(deny_value, deny);
    allow_perms.set_table();
    deny_perms.set_table();
}

discord::PermissionOverwrites::PermissionOverwrites(snowflake object_id, int object_type)
    : object_type{ object_type }, object_id{ object_id } {
    allow_perms = PermissionOverwrite(0, permission_type::allow);
    deny_perms = PermissionOverwrite(0, permission_type::deny);
}

std::pair<int, int> discord::PermissionOverwrites::get_values() const {
    return { allow_perms.value, deny_perms.value };
}

discord::PermissionOverwrites& discord::PermissionOverwrites::add_permission(std::string const& name, int allow_type) {
    if (allow_type == allow_perms.allow_type) {
        allow_perms.add_permission(name);
    } else if (allow_type == deny_perms.allow_type) {
        deny_perms.add_permission(name);
    }

    this->base_permissions = 0;
    this->base_permissions &= ~deny_perms.value;
    this->base_permissions |= allow_perms.value;
    return *this;
}

nlohmann::json discord::PermissionOverwrites::to_json() const {
    auto t = object_type == role ? "role" : "member";
    auto vals = get_values();
    return nlohmann::json({ { "allow", vals.first },
                            { "deny", vals.second },
                            { "type", t },
                            { "id", object_id } });
}