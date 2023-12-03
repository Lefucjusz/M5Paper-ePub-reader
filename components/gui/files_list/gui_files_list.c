#include "gui_files_list.h"
#include "lvgl.h"
#include <dirent.h>
#include <esp_log.h>

void gui_files_list_create(void)
{
    lv_obj_t *list = lv_list_create(lv_scr_act());
    lv_obj_set_size(list, GUI_FILES_LIST_WIDTH, GUI_FILES_LIST_HEIGHT);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, GUI_FILES_LIST_OFFSET_X);
    lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLL_ELASTIC);

    struct dirent *entry;
    DIR *dir = opendir("/sdcard");
    if (dir == NULL) {
        ESP_LOGE("GUI_FILES_LIST", "opendir failed");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        lv_obj_t *list_btn = lv_list_add_btn(list, entry->d_type == DT_DIR ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE, entry->d_name);
        lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
        lv_obj_set_style_pad_top(list_btn, 20, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(list_btn, 20, LV_PART_MAIN);
        lv_obj_t *list_btn_label = lv_obj_get_child(list_btn, lv_obj_get_child_cnt(list_btn) - 1); // Label is created as a last child
        lv_label_set_long_mode(list_btn_label, LV_LABEL_LONG_WRAP); // Disable scrolling, just wrap the text
    }

    closedir(dir);
}
