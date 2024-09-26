#include "ErrorPopup.hpp"
#include "style/Style.hpp"
#include "Colors.hpp"
#include "Fonts.h"
#include <stdio.h>

namespace gui
{
    auto createErrorPopup(const std::string &message) -> void
    {
        auto msgbox = lv_msgbox_create(nullptr, style::error::title, message.c_str(), nullptr, true);
        lv_obj_set_style_border_width(msgbox, style::error::borderWidth, LV_PART_MAIN);
        
        auto title = lv_msgbox_get_title(msgbox);
        lv_obj_set_height(title, style::error::titleHeight);
        lv_obj_set_style_pad_left(title, style::error::titlePaddingLeft, LV_PART_MAIN);
        lv_obj_set_style_bg_color(title, colors::black, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(title, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_text_font(title, &gui_montserrat_medium_36, LV_PART_MAIN);
        lv_obj_set_style_text_color(title, colors::white, LV_PART_MAIN);

        auto text = lv_msgbox_get_text(msgbox);
        lv_obj_set_style_pad_top(text, style::error::textPaddingTop, LV_PART_MAIN);
        lv_obj_set_style_text_font(text, &gui_montserrat_medium_28, LV_PART_MAIN);

        auto button = lv_msgbox_get_close_btn(msgbox);
        lv_obj_set_style_bg_color(button, colors::black, LV_PART_MAIN);
        lv_obj_set_style_text_color(button, colors::white, LV_PART_MAIN);

        lv_obj_set_height(msgbox, style::error::height);
        lv_obj_center(msgbox);
    }
}
