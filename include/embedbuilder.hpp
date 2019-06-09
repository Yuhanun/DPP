#pragma once
#include <nlohmann/json.hpp>
#include "discord.hpp"
#include <color.hpp>

namespace discord {

    class EmbedBuilder {
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
        nlohmann::json embed;
    };
};  // namespace discord