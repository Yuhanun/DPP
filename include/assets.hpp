#pragma once
#include <string>
#include <cstdint>

namespace discord {

    typedef int64_t snowflake;

    class Asset {
        /**
         * @class Assets are basically images that are used for fetching content of images etc etc.
         * 
         * ```cpp
         *      void do_something(discord::Asset const& asset) {
         *          std::cout << asset.url << std::endl;
         *      }
         * ```
         */
    public:
        std::string byte_arr; /**< Byte array of the raw image data. */
        bool gotten_data; /**< Whether the \ref byte_arr has already been gotten from the server */
        snowflake obj_id; /**< Snowflake of the Object this asset belongs to */
        std::string url; /**< Url of the Asset */
        int asset_type; /**< Type of the asset */
        bool _animated; /**< Whether the asset is animated */

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