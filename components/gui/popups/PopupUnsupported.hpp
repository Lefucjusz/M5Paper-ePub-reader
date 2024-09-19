#pragma once

#include <filesystem>

namespace gui
{
    #define GUI_UNSUPPORTED_POPUP_TITLE LV_SYMBOL_WARNING" Unsupported file" // TODO constexprs
    #define GUI_UNSUPPORTED_POPUP_MAX_MSG_LENGTH 128
    #define GUI_UNSUPPORTED_POPUP_BORDER_WIDTH 2
    #define GUI_UNSUPPORTED_POPUP_HEIGHT 200
    #define GUI_UNSUPPORTED_POPUP_TITLE_HEIGHT 42

    auto createPopupUnsupported(const std::filesystem::path &filename) -> void;
}
