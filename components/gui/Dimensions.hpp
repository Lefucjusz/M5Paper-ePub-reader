#pragma once

#include <eink.h>

namespace gui::style
{
    namespace display
    {
        /* Total screen size */
        inline constexpr auto width = EINK_DISPLAY_HEIGHT;
        inline constexpr auto height = EINK_DISPLAY_WIDTH;
    }
    
    namespace margins
    {
        /* Vertical margins to compensate for casing covering the screen */
        inline constexpr auto verticalLeft = 5;
        inline constexpr auto verticalRight = 5;
        inline constexpr auto verticalTotal = verticalLeft + verticalRight;

        /* Horizontal margins to compensate for casing covering the screen */
        inline constexpr auto horizontalTop = 10;
        inline constexpr auto horizontalBottom = 10;
        inline constexpr auto horizontalTotal = horizontalTop + horizontalBottom;
    }

    namespace status_bar
    {
        /* Status bar dimensions */
        inline constexpr auto width = display::width;
        inline constexpr auto height = 36;
    }

    namespace main_area
    {
        /* Main area dimensions */
        inline constexpr auto width = display::width - margins::verticalTotal;
        inline constexpr auto height = display::height - (status_bar::height + margins::horizontalTotal);
        inline constexpr auto minX = margins::verticalLeft;
        inline constexpr auto maxX = minX + width;
        inline constexpr auto minY = status_bar::height + margins::horizontalTop;
        inline constexpr auto maxY = minY + height;
    }
}
