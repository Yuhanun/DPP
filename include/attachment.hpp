#pragma once
#include "discord.hpp"

namespace discord {
    class Attachment {
    public:
        Attachment() = default;
        Attachment(nlohmann::json const);

        int size;
        int width;
        int height;
        snowflake id;
        std::string url;
        std::string filename;
        std::string proxy_url;
    };
} // namespace discord