#include "gui_unsupported_popup.h"
#include "gui_colors.h"
#include <stdio.h>

void gui_unsupported_popup_create(const char *filename)
{
    char msg_buffer[GUI_UNSUPPORTED_POPUP_MAX_MSG_LENGTH];
    snprintf(msg_buffer, sizeof(msg_buffer), "File '%s' has unsupported format!", filename);

    lv_obj_t *msgbox = lv_msgbox_create(NULL, GUI_UNSUPPORTED_POPUP_TITLE, msg_buffer, NULL, true);
    lv_obj_set_style_border_width(msgbox, GUI_UNSUPPORTED_POPUP_BORDER_WIDTH, LV_PART_MAIN);
    
    lv_obj_set_height(lv_msgbox_get_title(msgbox), GUI_UNSUPPORTED_POPUP_TITLE_HEIGHT);
    lv_obj_set_style_text_font(lv_msgbox_get_title(msgbox), &lv_font_montserrat_36, LV_PART_MAIN);
    lv_obj_set_style_bg_color(lv_msgbox_get_title(msgbox), GUI_COLOR_LIGHT_GREY, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_msgbox_get_title(msgbox), LV_OPA_COVER, LV_PART_MAIN); // TODO fix spacing around

    lv_obj_set_style_text_font(lv_msgbox_get_text(msgbox), &lv_font_montserrat_28, LV_PART_MAIN); // TODO center this text

    lv_obj_set_height(msgbox, GUI_UNSUPPORTED_POPUP_HEIGHT);
    lv_obj_center(msgbox);
}
