#pragma once

namespace D2Color
{
enum ColorCode : unsigned char
{
    White = 0x2F,
    Grey,
    Red,
    Green,
    Blue,
    Gold,
    DarkGrey,
    Black,
    Tan,
    Orange,
    Yellow,
    DarkGreen,
    Purple,
};

extern const std::string header;
extern const std::vector<std::pair<ColorCode, std::string>> readableColors;

extern decltype(readableColors)::value_type readableColor(char colorCode);
extern void convertToReadableColors(std::string& s) noexcept;
}
