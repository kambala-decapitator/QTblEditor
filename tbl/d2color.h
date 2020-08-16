#pragma once

#include "common/tbl.h"

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

extern const std::string TBL_EXPORT header;
extern const std::vector<std::pair<ColorCode, std::string>> TBL_EXPORT readableColors;

extern decltype(readableColors)::value_type TBL_EXPORT readableColor(char colorCode);
extern void TBL_EXPORT convertToReadableColors(std::string& s) noexcept;
}
