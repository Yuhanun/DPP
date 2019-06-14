#pragma once
#include "nlohmann/json.hpp"
#include <cstdint>

namespace discord {

    typedef int64_t snowflake;

    class Attachment {
        /**
         * @class An attachment is a file sent with a discord::Message
         */
    public:
        Attachment() = default;
        Attachment(nlohmann::json const);

        int size; /**< Size of the attachment */
        int width; /**< Width of the attachment */
        int height; /**< Height of the attachment */
        snowflake id; /**< Id of attachment */
        std::string url; /**< Url to attachment */
        std::string filename; /**< Filename of attachment */
        std::string proxy_url; /**< Proxied url of file */
    };
}  // namespace discord