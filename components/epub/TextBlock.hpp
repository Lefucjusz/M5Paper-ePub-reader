#pragma once

#include <string>

enum class Font
{
    Normal,
    Bold
};

struct TextBlock
{
    std::string text;
    Font font;
};
