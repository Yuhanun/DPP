#pragma once
#include "object.hpp"
#include "user.hpp"

namespace discord {

    typedef int64_t snowflake;

    class Emoji : public Object {
        /**
         * @brief Class which represents an Emoji.
         */
    public:
        Emoji() = default;
        Emoji(nlohmann::json);
        operator std::string();

    public:
        bool managed;        /**< Whether or not this emoji is managed by an integration */
        bool animated;       /**< Whether or not this emoji is animated */
        bool is_custom;      /**< Whether or not this is a custom emoji */
        bool require_colons; /**< Whether or not this emoji requires colons */

        snowflake id;                                      /**< The snowflake, id, of this emoji */
        std::string name;                                  /**< The name of this emoji */
        discord::User user;                                /**< User that created this emoji */
        discord::Asset image;                              /**< The Asset object that represents the image of this emoji */
        std::vector<std::shared_ptr<discord::Role>> roles; /**< Roles this emoji is whitelisted to */
    };
}  // namespace discord