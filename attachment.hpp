#pragma once
#include <discord.hpp>

discord::Attachment::Attachment(nlohmann::json const data)
    : size{ data["size"] },
      width{ get_value(data, "width", 0) },
      height{ get_value(data, "height", 0) },
      id{ to_sf(data["id"]) },
      url{ data["url"] },
      filename{ data["filename"] },
      proxy_url{ data["proxy_url"] } {
}