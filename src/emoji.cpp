#include "emoji.hpp"
#include "guild.hpp"
#include "object.hpp"
#include "role.hpp"
#include "user.hpp"
#include "utility.hpp"

discord::Emoji::Emoji(nlohmann::json event)
    : discord::Object(to_sf(get_value(event, "id", "0"))) {
    name = event["name"];

    if (event.contains("roles")) {
        for (auto const& each : event["roles"]) {
            [&]() {
                std::shared_ptr<discord::Role> rl;
                auto role_id = to_sf(each);
                for (auto const& gld : discord::detail::bot_instance->guilds) {
                    for (auto const& role : gld->roles) {
                        if (role->id == role_id) {
                            roles.push_back(role);
                            return;
                        }
                    }
                }
            }();
        }
    }

    if (event.contains("user")) {
        user = discord::User{ event["user"] };
    }

    is_custom = event.contains("require_colons") && event.contains("animated");
    require_colons = get_value(event, "require_colons", false);
    managed = get_value(event, "managed", false);
    animated = get_value(event, "animated", false);

    image = discord::Asset{};
    image.url = image_url_from_type(custom_emoji, id, "", animated);
    image.obj_id = id;
    image.asset_type = custom_emoji;
    image._animated = animated;
}

discord::Emoji::operator std::string() {
    return format("<%:%:%>", animated ? "a" : "", name, id);
}
