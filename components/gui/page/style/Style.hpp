#pragma once

#include "Dimensions.hpp"

namespace gui::style
{
    inline constexpr auto marginTop = 2;
    inline constexpr auto width = main_area::width;
    inline constexpr auto height = main_area::height - marginTop;
    inline constexpr auto offsetY = main_area::minY + marginTop;
    inline constexpr auto lineSpacing = 5;
}
