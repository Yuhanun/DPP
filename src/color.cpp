#include "color.hpp"

discord::Color::Color(int r, int g, int b)
    : r{ r }, g{ g }, b{ b } {
    /**
     * @brief Constructor to go from R G B to a discord::Color
     * 
     * ```cpp
     *      discord::Color color{255, 255, 255};
     * ```
     * 
     * @param[in] r Red value
     * @param[in] g Green value
     * @param[in] b Blue value
     * 
     */
    if (raw_color >= 0) {
        return;
    }
    raw_int = (r << 16) + (g << 8) + (b);
}

discord::Color::Color(int color_int) {
    /**
     * @brief Constructor to go from a raw color value to a discord::Color
     * 
     * ```cpp
     *      discord::Color color{0xffffff};
     * ```
     * 
     * @param[in] color_int The combined RGB value
     */
    r = (color_int & 0xFF000000) >> 24;
    g = (color_int & 0x00FF0000) >> 16;
    b = (color_int & 0x0000FF00) >> 8;
    raw_int = color_int;
}
