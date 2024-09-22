#pragma once

#include "Dimensions.hpp"

namespace gui::style
{
    inline constexpr auto marginTop = 2; 
    inline constexpr auto width = style::main_area::width;
    inline constexpr auto height = (style::main_area::height - marginTop);
    inline constexpr auto offsetY = (style::main_area::minY + marginTop);

    namespace button
    {
        inline constexpr auto padTop = 20;
        inline constexpr auto padBottom = 20;
    }
}
