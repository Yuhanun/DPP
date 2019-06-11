#include "embedbuilder.hpp"

discord::EmbedBuilder::EmbedBuilder(nlohmann::json event)
    : embed{ event } {
    /**
     * @brief Constructs a discord::Embed from an embed json
     * 
     * @param[in] event Json to construct from
     */
}

discord::EmbedBuilder::EmbedBuilder()
    : embed{ nlohmann::json({}) } {
    /**
     * @brief Constructs a clean embed
     */
}

discord::EmbedBuilder &discord::EmbedBuilder::set_title(std::string const &title) {
    /**
     * @brief Adds a title to embed
     * 
     * ```cpp
     *      discord::EmbedBuilder().set_title("Hello!");
     * ```
     * 
     * @param[in] title String to set the title to
     * 
     * Overwrites any old title.
     * 
     * @return Reference to the original discord::EmbedBuilder
     */
    embed["title"] = title;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_description(std::string const &desc) {
    /**
     * @brief Adds a description to embed
     * 
     * ```cpp
     *      discord::EmbedBuilder().set_description("Hello!");
     * ```
     * 
     * @param[in] desc String to set the description to
     * 
     * Overwrites any old description.
     * 
     * @return Reference to the original discord::EmbedBuilder
     */
    embed["description"] = desc;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_url(std::string const &url) {
    /**
     * @brief Adds a url to an embed.
     * 
     * ```cpp
     *      discord::EmbedBuilder().set_url("http://www.discordapp.com/");
     * ```
     * 
     * @param[in] url String to set the url to
     * 
     * Overwrites any old url
     * 
     * @return Reference to the original discord::EmbedBuilder
     */
    embed["url"] = url;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_timestamp(std::string const &iso_8601) {
    /**
     * @brief Adds a timestamp to an embed
     * 
     * ```cpp
     *      discord::EmbedBuilder().set_timestamp("2019-06-11T21:41:13.943644:00+00");
     * ```
     * 
     * @param[in] iso_8601 String timestamp in format "2019-06-11T21:41:13.943644:00+00" to set the timestamp to, will be replaced with a discord::datetime later
     * 
     * Overwrites any old timestamp
     * 
     * @return Reference to the original discord::EmbedBuilder
     */
    embed["timestamp"] = iso_8601;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_color(Color c) {
    /**
     * @brief Adds a timestamp to an embed
     * 
     * ```cpp
     *      discord::EmbedBuilder().set_color({255, 255, 255});
     * ```
     * 
     * @param[in] c discord::Color object to set the color to
     * 
     * Overwrites any old color
     * 
     * @return Reference to the original discord::EmbedBuilder
     */
    embed["color"] = c.raw_int;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_footer(std::string const &text, std::string const &icon_url) {
    /**
     * @brief Adds a footer to an embed
     * 
     * ```cpp
     *      discord::EmbedBuilder().set_footer("Hello!", "http://www.image.png/");
     * ```
     * 
     * @param[in] text String of text to set on the footer
     * @param[in] icon_url Url to set the footer icon to
     * 
     * Overwrites any old footer
     * 
     * @return Reference to the original discord::EmbedBuilder
     */
    embed["footer"] = nlohmann::json({});
    if (icon_url != "") {
        embed["footer"]["icon_url"] = icon_url;
    }
    embed["footer"]["text"] = text;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_image(std::string const &url, int w, int h) {
    /**
     * @brief Adds an image to an embed
     * 
     * ```cpp
     *      discord::EmbedBuilder().set_image("http://www.image.png/", 500, 500);
     * ```
     * 
     * @param[in] icon_url Url to set the image to
     * @param[in] w Desired width of the image 
     * @param[in] h Desired height of the image
     * 
     * Overwrites any old image
     * 
     * @return Reference to the original discord::EmbedBuilder
     */
    embed["image"] = nlohmann::json({ { "url", url } });
    if (w != -1) {
        embed["image"]["width"] = w;
    }
    if (h != -1) {
        embed["image"]["height"] = h;
    }
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_thumbnail(std::string const &url, int w, int h) {
    /**
     * @brief Adds a thumbnail to an embed
     * 
     * ```cpp
     *      discord::EmbedBuilder().set_thumbnail("http://www.image.png/", 500, 500);
     * ```
     * 
     * @param[in] url Url to set the thumbnail icon to
     * @param[in] w Desired width of the thumnail image
     * @param[in] h Desired height of the thumnail image
     * 
     * Overwrites any old thumbnail
     * 
     * @return Reference to the original discord::EmbedBuilder
     */
    embed["thumbnail"] = nlohmann::json({ { "url", url } });
    if (w != -1) {
        embed["thumbnail"]["width"] = w;
    }
    if (h != -1) {
        embed["thumbnail"]["height"] = h;
    }

    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_video(std::string const &url, int w, int h) {
    /**
     * @brief Adds a video to an embed
     * 
     * ```cpp
     *      discord::EmbedBuilder().set_video("http://www.youtube.com/video.mp4", 500, 500);
     * ```
     * 
     * @param[in] url Url to set the video to
     * @param[in] w Desired width of the video
     * @param[in] h Desired height of the video
     * 
     * Overwrites any old video
     * 
     * @return Reference to the original discord::EmbedBuilder
     */
    embed["video"] = nlohmann::json({});
    if (w != -1) {
        embed["video"]["width"] = w;
    }
    if (h != -1) {
        embed["video"]["height"] = h;
    }

    embed["video"]["url"] = url;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::set_author(std::string const &name, std::string const &url, std::string const &icon_url) {
    /**
     * @brief Adds a video to an embed
     * 
     * ```cpp
     *      discord::EmbedBuilder().set_author("Yuhanun", "http://www.discordapp.com/", "http://www.image.png");
     * ```
     * 
     * @param[in] name Name of the author
     * @param[in] url Url that the author's name links to
     * @param[in] icon_url Url of the image that will be used for the icon
     * 
     * Overwrites any old author
     * 
     * @return Reference to the original discord::EmbedBuilder
     */
    embed["author"] = nlohmann::json({});
    if (!icon_url.empty()) {
        embed["author"]["icon_url"] = icon_url;
    }
    if (url != "") {
        embed["author"]["url"] = url;
    }

    embed["author"]["name"] = name;
    return *this;
}

discord::EmbedBuilder &discord::EmbedBuilder::add_field(std::string const &name, std::string const &value, bool in_line) {
    /**
     * @brief Adds a field to an embed
     * 
     * ```cpp
     *      discord::EmbedBuilder().add_field("Field name", "Field content", true);
     * ```
     * 
     * @param[in] name Name of the field
     * @param[in] value Value of the field
     * @param[in] in_line Whether or not the field should be inlined
     * 
     * @return Reference to the original discord::EmbedBuilder
     */
    if (!embed.contains("fields")) {
        embed["fields"] = nlohmann::json::array();
    }
    embed["fields"].push_back(nlohmann::json({ { "name", name },
                                               { "value", value },
                                               { "inline", in_line } }));
    return *this;
}

nlohmann::json discord::EmbedBuilder::to_json() const {
    /**
     * @brief Returns a copy to the json object which represents the created embed
     * 
     * ```cpp
     *      discord::EmbedBuilder().set_title("Test").to_json(); // {"title", "Test"}
     * ```
     * 
     * @return Returns an nlohmann::json
     */
    return embed;
}

discord::EmbedBuilder::operator nlohmann::json() {
    /**
     * @brief Returns the json object which represents the created embed
     * 
     * ```cpp
     *      static_cast<nlohmann::json>(discord::EmbedBuilder().set_title("Test")) // {"title", "Test"}
     * ```
     * 
     * Keep in mind this is an implicit conversion.
     * 
     * @return Returns an nlohmann::json
     */
    return embed;
}