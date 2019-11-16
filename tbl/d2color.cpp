#include "d2color.h"

namespace D2Color
{
const std::string header{static_cast<char>(0xC3), static_cast<char>(0xBF), 0x63}; // first 2 bytes are U+00FF: ÿ
const std::vector<std::pair<ColorCode, std::string>> readableColors = {
    {White,     "\\white;"},
    {Grey,      "\\grey;"},
    {Red,       "\\red;"},
    {Green,     "\\green;"},
    {Blue,      "\\blue;"},
    {Gold,      "\\gold;"},
    {DarkGrey,  "\\dgrey;"},
    {Black,     "\\black;"},
    {Tan,       "\\tan;"},
    {Orange,    "\\orange;"},
    {Yellow,    "\\yellow;"},
    {DarkGreen, "\\dgreen;"},
    {Purple,    "\\purple;"},
};

decltype(readableColors)::value_type readableColor(char colorCode)
{
    return readableColors.at(colorCode - White);
}

void convertToReadableColors(std::string& s) noexcept
{
    std::string::size_type pos = 0;
    for (;;)
    {
        pos = s.find(header, pos);
        if (pos == std::string::npos)
            return;

        auto colorStart = pos;
        pos += header.length();
        try
        {
            auto colorString = readableColor(s.at(pos)).second;
            s.replace(colorStart, header.length() + 1, colorString);
            pos = colorStart + colorString.length();
        }
        catch (...) {}
    }
}
}
