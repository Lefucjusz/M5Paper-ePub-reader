#pragma once

#include "Dimensions.hpp"

namespace gui::style
{
    namespace top_bar
    {
        inline constexpr auto marginTop = 2;
        inline constexpr auto width = main_area::width;
        inline constexpr auto height = 60;
        inline constexpr auto offsetY = main_area::minY + marginTop;
    }

    namespace back_button
    {
        inline constexpr auto offsetX = 2;
        inline constexpr auto height = 50;
    }

    namespace list
    {
        inline constexpr auto width = main_area::width;
        inline constexpr auto height = main_area::height - (top_bar::height + top_bar::marginTop);

        namespace button
        {
            inline constexpr auto padTop = 20;
            inline constexpr auto padBottom = 20;
        }
    }
}
