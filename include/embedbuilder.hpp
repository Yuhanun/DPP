#pragma once
#include <nlohmann/json.hpp>
#include "color.hpp"

namespace discord {

    class EmbedBuilder {
        /**
     * @class
     * @brief Class used for sending embeds
     * 
     * ```cpp
     *      ctx.send(EmbedBuilder().set_title("Hello!")
     *                             .set_description("My name is Yuhanun")
     *                             .set_url("http://www.discordapp.com")
     *                             .set_timestamp("2019-06-11T21:41:13.943644:00+00")
     *                             .set_color(discord::Color{0xffffff})
     *                             .set_footer("By Yuhanun", "http://www.image.png")
     *                             .set_image("http://www.image.png")
     *                             .set_thumbnail("http://www.image.png")
     *                             .set_video("http://www.youtube.com/video.mp4")
     *                             .set_author("Yuhanun", "http://www.discordapp.com/", "http://www.image.png")
     *                             .add_field("Field one", "hello")
     *                             .add_field("Inlined field", "Inlined?!?!", true));
     * ```
     * 
     * Is also used for representing embeds.
     */
    public:
        EmbedBuilder(nlohmann::json);
        EmbedBuilder();
        EmbedBuilder& set_title(std::string const&);
        EmbedBuilder& set_description(std::string const&);
        EmbedBuilder& set_url(std::string const&);
        EmbedBuilder& set_timestamp(std::string const&);
        EmbedBuilder& set_color(const discord::Color);
        EmbedBuilder& set_footer(std::string const&, std::string const&);
        EmbedBuilder& set_image(std::string const&, const int = -1, const int = -1);
        EmbedBuilder& set_thumbnail(std::string const&, const int = -1, const int = -1);
        EmbedBuilder& set_video(std::string const&, const int = -1, const int = -1);
        EmbedBuilder& set_author(std::string const&, std::string const&, std::string const&);
        EmbedBuilder& add_field(std::string const&, std::string const&, const bool = false);
        nlohmann::json to_json() const;
        operator nlohmann::json();

    private:
        nlohmann::json embed; /**< JSON representation of the created embed */
    };
} // namespace discord
