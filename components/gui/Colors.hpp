#pragma once

#include "lvgl.h"

namespace gui::colors
{
    /* All refreshable by DU4 */
    inline const auto black = lv_color_black();
    inline const auto darkGrey = lv_color_make(95, 96, 96);
    inline const auto lightGrey = lv_color_make(180, 180, 180);
    inline const auto white = lv_color_white();
}
