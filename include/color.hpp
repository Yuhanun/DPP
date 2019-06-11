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
        int raw_int;
        int r;
        int g;
        int b;
    };
} // namespace discord