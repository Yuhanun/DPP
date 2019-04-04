#pragma once
namespace discord {
    class Color {
    public:
        Color()
            : r{0}, g{0}, b{0}, raw_int{0}
        {}
        
        Color(uint8_t r, uint8_t g, uint8_t b, int raw_color = -1)
            : r{r}, g{g}, b{b}
        {
            if (raw_color >= 0){ return; }
            raw_int = (r << 16) + (g << 8) + (b);
        }

        static Color from_integer(int color_int){
            uint8_t red = (color_int & 0xFF000000) >> 24;
            uint8_t green = (color_int & 0x00FF0000) >> 16;
            uint8_t blue = (color_int & 0x0000FF00) >> 8;
            return Color{red, green, blue, color_int};
        }
    public:
        int raw_int;
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
};
