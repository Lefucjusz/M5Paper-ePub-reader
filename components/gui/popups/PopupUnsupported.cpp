#include "PopupUnsupported.hpp"
#include "Colors.hpp"
#include "Fonts.h"
#include <stdio.h>

namespace gui
{
    auto createPopupUnsupported(const std::filesystem::path &filename) -> void
    {
        const auto &message = "File " + filename.string() + " has unsupported format!";

        auto msgbox = lv_msgbox_create(NULL, GUI_UNSUPPORTED_POPUP_TITLE, message.c_str(), nullptr, true);
        lv_obj_set_style_border_width(msgbox, GUI_UNSUPPORTED_POPUP_BORDER_WIDTH, LV_PART_MAIN);
        
        lv_obj_set_height(lv_msgbox_get_title(msgbox), GUI_UNSUPPORTED_POPUP_TITLE_HEIGHT);
        lv_obj_set_style_text_font(lv_msgbox_get_title(msgbox), &gui_montserrat_medium_36, LV_PART_MAIN);
        lv_obj_set_style_bg_color(lv_msgbox_get_title(msgbox), colors::lightGrey, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(lv_msgbox_get_title(msgbox), LV_OPA_COVER, LV_PART_MAIN); // TODO fix spacing around

        lv_obj_set_style_text_font(lv_msgbox_get_text(msgbox), &gui_montserrat_medium_28, LV_PART_MAIN); // TODO center this text

        lv_obj_set_height(msgbox, GUI_UNSUPPORTED_POPUP_HEIGHT);
        lv_obj_center(msgbox);
    }
}
