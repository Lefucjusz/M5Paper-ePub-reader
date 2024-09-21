#pragma once

#include "Dimensions.hpp"
#include <filesystem>

namespace gui
{
    #define GUI_TOC_BAR_MARGIN_TOP 2
    #define GUI_TOC_BAR_WIDTH GUI_MAIN_AREA_WIDTH
    #define GUI_TOC_BAR_HEIGHT 60
    #define GUI_TOC_BAR_OFFSET_Y (GUI_MAIN_AREA_MIN_Y + GUI_TOC_BAR_MARGIN_TOP)

    #define GUI_TOC_BACK_BUTTON_OFFSET_X -4
    #define GUI_TOC_BACK_BUTTON_HEIGHT 50

    #define GUI_TOC_LIST_WIDTH GUI_MAIN_AREA_WIDTH
    #define GUI_TOC_LIST_HEIGHT (GUI_MAIN_AREA_HEIGHT - (GUI_TOC_BAR_HEIGHT + GUI_TOC_BAR_MARGIN_TOP))
    #define GUI_TOC_LIST_BUTTON_PAD_TOP 20
    #define GUI_TOC_LIST_BUTTON_PAD_BOTTOM 20

    auto tocListViewCreate(const std::filesystem::path &epubPath) -> void;
}
