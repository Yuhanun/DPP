#pragma once
#include "discord.hpp"

namespace discord {

    typedef int64_t snowflake;

    class Asset {
    public:
        std::string byte_arr;
        bool gotten_data;
        snowflake obj_id;
        std::string url;
        int asset_type;
        bool _animated;

        Asset() = default;
        Asset(std::string const&, int, bool = false, snowflake = 0);

        int type() const;
        std::string read();
        size_t len() const;
        bool animated() const;
        std::string hash() const;
        snowflake object_id() const;
        void save(std::string const&);
        explicit operator bool() const;
        explicit operator std::string() const;
        bool operator==(Asset const&) const;
        bool operator!=(Asset const&) const;
    };
} // namespace discord