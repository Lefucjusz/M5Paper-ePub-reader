#pragma once

#include <lvgl.h>

namespace gui::style
{
    namespace error
    {
        inline constexpr auto title = LV_SYMBOL_WARNING " Error";
        inline constexpr auto titleHeight = 42;
        inline constexpr auto titlePaddingLeft = 3;
        inline constexpr auto height = 200;
        inline constexpr auto borderWidth = 2;
        inline constexpr auto textPaddingTop = 4;
    }
};
