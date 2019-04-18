#pragma once

#include "discord.hpp"

discord::EmbedBuilder::EmbedBuilder() {
    embed = json({});
}

discord::EmbedBuilder &
discord::EmbedBuilder::set_title(std::string const &title) {
    embed["title"] = title;
    return *this;
}

discord::EmbedBuilder &
discord::EmbedBuilder::set_description(std::string const &desc) {
    embed["description"] = desc;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_url(std::string const &url) {
    embed["url"] = url;
    return *this;
}

discord::EmbedBuilder &
discord::EmbedBuilder::set_timestamp(std::string const &iso_8601) {
    embed["timestamp"] = iso_8601;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_color(Color c) {
    embed["color"] = c.raw_int;
    return *this;
}

discord::EmbedBuilder &
discord::EmbedBuilder::set_footer(std::string const &text,
                                  std::string const &icon_url) {
    embed["footer"] = json({});
    if (icon_url != "") {
        embed["footer"]["icon_url"] = icon_url;
    }
    embed["footer"]["text"] = text;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_image(std::string const &url,
                                                        int w,
                                                        int h) {
    embed["image"] = json({});
    if (w != -1) {
        embed["image"]["width"] = w;
    }
    if (h != -1) {
        embed["image"]["height"] = h;
    }

    embed["image"]["url"] = url;
    return *this;
}

discord::EmbedBuilder &
discord::EmbedBuilder::set_thumbnail(std::string const &url, int w, int h) {
    embed["thumbnail"] = json({});
    if (w != -1) {
        embed["thumbnail"]["width"] = w;
    }
    if (h != -1) {
        embed["thumbnail"]["height"] = h;
    }

    embed["thumbnail"]["url"] = url;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_video(std::string const &url,
                                                        int w,
                                                        int h) {
    embed["video"] = json({});
    if (w != -1) {
        embed["video"]["width"] = w;
    }
    if (h != -1) {
        embed["video"]["height"] = h;
    }

    embed["video"]["url"] = url;
    return *this;
}

discord::EmbedBuilder &
discord::EmbedBuilder::set_author(std::string const &name,
                                  std::string const &url,
                                  std::string const &icon_url) {
    embed["author"] = json({});
    if (icon_url != "") {
        embed["author"]["icon_url"] = icon_url;
    }
    if (url != "") {
        embed["author"]["url"] = url;
    }

    embed["author"]["name"] = url;
    return *this;
}

discord::EmbedBuilder &
discord::EmbedBuilder::add_field(std::string const &name,
                                 std::string const &value,
                                 bool in_line) {
    if (!embed.contains("fields")) {
        embed["fields"] = json::array();
    }
    json field{};
    field["name"] = name;
    field["value"] = value;
    embed["fields"].push_back(field);
    return *this;
}

nlohmann::json &discord::EmbedBuilder::to_json() {
    return embed;
}