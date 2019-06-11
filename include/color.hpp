#pragma once

namespace discord {
    class Color {
    /**
     * @class
     * @brief Color class used for colors in embeds, roles etc.
     * 
     * ```cpp
     * discord::EmbedBuilder().set_color(discord::Color{255, 255, 255});
     * discord::EmbedBuilder().set_color(0xffffff);
     * ```
     * 
     */
    public:
        Color() = default;
        Color(int, int, int);
        Color(int);

    public:
        int raw_int; /**< Combined RGB value */
        int r; /**< Red value */
        int g; /**< Green value */
        int b; /**< Blue value */
    };
} // namespace discord