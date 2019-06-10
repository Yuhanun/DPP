#pragma once

namespace discord {
    class Color {
    public:
        Color() = default;
        Color(int, int, int, int = -1);
        Color(int);

    public:
        int raw_int;
        int r;
        int g;
        int b;
    };
} // namespace discord