#pragma once

#include "Dimensions.hpp"

namespace gui::style
{
    namespace border
    {
        inline constexpr auto width = 1;
    }

    namespace label
    {
        inline constexpr auto offsetX = margins::verticalRight;
        inline constexpr auto offsetY = 4;
    }

    namespace icon
    {
        inline constexpr auto offsetX = -10;
        inline constexpr auto offsetY = -3;
    }
}
