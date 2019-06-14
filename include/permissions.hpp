#pragma once
#include <string>
#include "unordered_map"
#include "nlohmann/json.hpp"

namespace discord {
    typedef int64_t snowflake;

    class PermissionOverwrite {
    public:
        PermissionOverwrite() = default;
        PermissionOverwrite(int, int);
        PermissionOverwrite& add_permission(std::string const&);

        bool has_permission(std::string const&);
        void calculate_value();
        void set_table();

        int value;
        int allow_type;

        std::unordered_map<std::string, int> ows;
    };

    class PermissionOverwrites {
    public:
        PermissionOverwrites() = default;
        PermissionOverwrites(int, int, snowflake, int);
        PermissionOverwrites(snowflake, int);
        // PermissionOverwrites(int, snowflake, int);

        PermissionOverwrites& add_permission(std::string const&, int);

        nlohmann::json to_json() const;
        std::pair<int, int> get_values() const;

        int object_type;

        snowflake object_id;

        PermissionOverwrite allow_perms;
        PermissionOverwrite deny_perms;
        int base_permissions;
    };
} // namespace discord