#pragma once
#include <discord.hpp>

discord::Attachment::Attachment(nlohmann::json const& data) {
    id = to_sf(data["id"]);
    filename = data["filename"];
    size = data["size"];
    url = data["url"];
    proxy_url = data["proxy_url"];

    height = get_value(data, "height", -1);
    width = get_value(data, "width", -1);
}