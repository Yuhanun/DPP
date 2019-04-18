#pragma once
#include "discord.hpp"

discord::Color::Color(int r, int g, int b, int raw_color) : r{r}, g{g}, b{b} {
  if (raw_color >= 0) {
    return;
  }
  raw_int = (r << 16) + (g << 8) + (b);
}

discord::Color::Color(int color_int) {
  r = (color_int & 0xFF000000) >> 24;
  g = (color_int & 0x00FF0000) >> 16;
  b = (color_int & 0x0000FF00) >> 8;
  raw_int = color_int;
}
