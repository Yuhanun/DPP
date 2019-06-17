#include <utility>

#include "utils.hpp"
#include "assets.hpp"

namespace discord {

    Asset::Asset(std::string const& hash_or_url, int asset_t, bool animat, snowflake some_id)
        : obj_id{ some_id }, asset_type{ asset_t }, _animated{ animat } {
        /**
         * @brief Constructs a discord::Asset object, shouldn't have to be used by a user
         */
        if (some_id == 0) {
            url = hash_or_url;
        } else {
            url = image_url_from_type(asset_t, some_id, hash_or_url, animat);
        }
    }

    std::string Asset::read() {
        /**
         * @brief Possibly fetches image data 
         * 
         * Will be a blocking call for a longer time if \ref gotten_data is false.
         * Otherwise instantly returns an std::string containing all the image data.
         * 
         * @return std::string Contains image data.
         */
        if (!gotten_data) {
            web::http::client::http_client client{ { this->url } };
            web::http::http_request msg{ methods::GET };

            for (auto const& each : get_default_headers()) {
                msg.headers().add(each.first, each.second);
            }

            client.request(msg).then([this](http_response resp_val) {
                gotten_data = true;
                this->byte_arr = resp_val.extract_utf8string(true).get();
            }).wait();
        }
        return byte_arr;
    }

    void Asset::save(std::string const& f_name) {
        /**
         * ```cpp
         *      void do_something(discord::Asset const& asset){
         *          asset.save("hello.txt");         
         *      }
         * ```
         * @param[in] f_name Filepath to store this Asset to.
         * @return void
         */
        std::ofstream file{ f_name, std::ios::binary };
        file << read();
    }

    int Asset::type() const {
        /**
         * @brief Returns the asset type that this asset is
         */
        return asset_type;
    }


    size_t Asset::len() const {
        /**
         * @brief Returns the length of the asset in bytes
         * 
         * The length is 0 if \ref gotten_data is false
         * 
         * @return size_t The length of the asset
         */
        return byte_arr.size();
    }

    bool Asset::animated() const {
        /**
         * @brief Returns true if asset is animated, otherwise false
         * 
         * @return bool Simply returns \ref _animated
         */
        return _animated;
    }

    std::string Asset::hash() const {
        /**
         * @brief Returns the has of the asset.
         * 
         * For now returns a default constructed string
         * 
         * @return std::string The has of the asset
         */
        return "";
    }

    snowflake Asset::object_id() const {
        /**
         * @brief Returns the snowflake, id, of the object that this Asset belongs to.
         * 
         * @return Snowflake The snowflake of the object this Asset belongs to.
         */
        return obj_id;
    }

    Asset::operator bool() const {
        /**
         * @brief true if url is not empty, false if url is empty.
         * 
         * ```cpp
         *      discord::Asset asset{};
         *      static_cast<bool>(asset); // false
         * ```
         * 
         * @return bool true if populated, false if empty
         */
        return !url.empty();
    }

    Asset::operator std::string() const {
        /**
         * @brief returns the URL of this asset, will return a default constructed string if no url could be found.
         * 
         *  ```cpp
         *      std::cout << static_cast<std::string>(my_asset) << std::endl;
         *  ```
         * 
         * @return std::string The URL of the asset
         */
        return url;
    }

    bool Asset::operator==(Asset const& rhs) const {
        /**
         * @brief true if the urls are the same, otherwise false
         * 
         * Keep in mind that if the urls are default constructed then this will return true
         * 
         * @return bool
         */
        return this->url == rhs.url;
    }

    bool Asset::operator!=(Asset const& rhs) const {
        /**
         * @brief false if the urls are the same, otherwise true
         * 
         * Keep in mind that if the urls are default constructed then this will return false
         * 
         * @return bool
         */
        return !(*this == rhs);
    }
}  // namespace discord
