#pragma once
#include <utility>

#include <utility.hpp>
#include "discord.hpp"

namespace discord {

    Asset::Asset(std::string const& hash_or_url, int asset_t, bool animat, snowflake some_id)
        : asset_type{ asset_t }, _animated{ animat } {
        if (some_id == 0) {
            url = hash_or_url;
        } else {
            url = image_url_from_type(asset_t, some_id, hash_or_url, animat);
        }
    }

    std::string Asset::read() {
        if (!gotten_data) {
            byte_arr = cpr::Get(cpr::Url{ url }).text;
            gotten_data = true;
        }
        return byte_arr;
    }

    void Asset::save(std::string const& f_name) {
        std::ofstream file{ f_name, std::ios::binary };
        if (!gotten_data) {
            file << read();
        } else {
            file << byte_arr;
        }
    }

    int Asset::type() const {
        return asset_type;
    }


    size_t Asset::len() const {
        return byte_arr.size();
    }

    bool Asset::animated() const {
        return _animated;
    }

    std::string Asset::hash() const {
        return "";
    }

    snowflake Asset::object_id() const {
        return obj_id;
    }

    Asset::operator bool() const {
        return !url.empty();
    }

    Asset::operator std::string() const {
        return url;
    }

    bool Asset::operator==(Asset const& rhs) const {
        return this->url == rhs.url;
    }

    bool Asset::operator!=(Asset const& rhs) const {
        return !(*this == rhs);
    }
}  // namespace discord
