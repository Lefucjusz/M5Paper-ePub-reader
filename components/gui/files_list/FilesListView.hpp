#pragma once

#include "Dimensions.hpp"
#include <filesystem>

namespace gui
{
    #define GUI_FILES_LIST_MARGIN_TOP 2 // TODO move to style
    #define GUI_FILES_LIST_WIDTH GUI_MAIN_AREA_WIDTH
    #define GUI_FILES_LIST_HEIGHT (GUI_MAIN_AREA_HEIGHT - GUI_FILES_LIST_MARGIN_TOP)
    #define GUI_FILES_LIST_OFFSET_Y (GUI_MAIN_AREA_MIN_Y + GUI_FILES_LIST_MARGIN_TOP)
    #define GUI_FILES_LIST_BUTTON_PAD_TOP 20
    #define GUI_FILES_LIST_BUTTON_PAD_BOTTOM 20

    auto filesListViewCreate(const std::filesystem::path &path) -> void;
}
