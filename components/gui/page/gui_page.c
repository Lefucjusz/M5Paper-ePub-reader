#include "gui_page.h"
#include "lvgl.h"
#include "epub_section.h"
#include <esp_log.h>

static lv_obj_t *obj = NULL;

static vec_void_t *cur_sec = NULL;

static size_t par_cnt = 0;

void cb(lv_event_t *event)
{
    ESP_LOGW("", "Got event %d", lv_event_get_code(event));
    ESP_LOGW("", "Direction: %s", lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT ? "left" : "right or different");
    gui_page_update(NULL);
}

void gui_page_create(vec_void_t *section)
{
    cur_sec = section;

    obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, GUI_PAGE_WIDTH, GUI_PAGE_HEIGHT);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, GUI_PAGE_OFFSET_Y);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_event_cb(obj, cb, LV_EVENT_GESTURE, NULL);

    size_t i;
    epub_paragraph_t *par;
    vec_foreach(section, par, i) {
        lv_obj_t *label = lv_label_create(obj);
        lv_label_set_text(label, par->data);
        lv_obj_set_width(label, GUI_PAGE_WIDTH);
        lv_obj_set_style_pad_bottom(label, 0, LV_PART_MAIN);

        

        if (par->type == EPUB_FONT_NORMAL) {
            lv_obj_set_style_text_font(label, &lv_font_montserrat_28, LV_PART_MAIN);
        }
        else {
            lv_obj_set_style_text_font(label, &lv_font_montserrat_36, LV_PART_MAIN);
        }
        lv_obj_set_style_text_line_space(label, 5, LV_PART_MAIN);
        if (lv_obj_get_child_cnt(obj) > 1) {
            lv_obj_set_style_pad_top(label, 5, LV_PART_MAIN);
            lv_obj_align_to(label, lv_obj_get_child(obj, lv_obj_get_child_cnt(obj) - 2), LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        }

        if (i > 2) {
            par_cnt = i;
            break;
        }
    }
}

void gui_page_update(const char *text)
{
    lv_obj_clean(obj);

    size_t i = par_cnt;
    for (; i < par_cnt + 1; ++i) {
        epub_paragraph_t *par = cur_sec->data[i];

        lv_obj_t *label = lv_label_create(obj);
        lv_label_set_text(label, par->data);
        lv_obj_set_width(label, GUI_PAGE_WIDTH);
        lv_obj_set_style_pad_bottom(label, 0, LV_PART_MAIN);

        

        if (par->type == EPUB_FONT_NORMAL) {
            lv_obj_set_style_text_font(label, &lv_font_montserrat_28, LV_PART_MAIN);
        }
        else {
            lv_obj_set_style_text_font(label, &lv_font_montserrat_36, LV_PART_MAIN);
        }
        lv_obj_set_style_text_line_space(label, 5, LV_PART_MAIN);
        if (lv_obj_get_child_cnt(obj) > 1) {
            lv_obj_set_style_pad_top(label, 5, LV_PART_MAIN);
            lv_obj_align_to(label, lv_obj_get_child(obj, lv_obj_get_child_cnt(obj) - 2), LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        }
    }

    par_cnt = i;


    // const char *test[] = {
    //     "This is the first paragraph in the testset, probably the longest one, as I wrote quite a lot of words here to make sure.",
    //     "This is second paragraph, which should be displayed with slightly bigger font.",
    //     "This is third paragraph, written with the smallest font."
    // };

    // for (size_t i = 0; i < 3; ++i) {
    //     lv_obj_t *label = lv_label_create(obj);
    //     lv_label_set_text(label, test[i]);
    //     lv_obj_set_width(label, GUI_PAGE_WIDTH);
    //     lv_obj_set_style_pad_bottom(label, 0, LV_PART_MAIN);
    //     if (i == 0) {
    //         lv_obj_set_style_text_font(label, &lv_font_montserrat_28, LV_PART_MAIN);
    //     }
    //     else if (i == 1) {
    //         lv_obj_set_style_text_font(label, &lv_font_montserrat_32, LV_PART_MAIN);
    //     }
    //     lv_obj_set_style_text_line_space(label, 5, LV_PART_MAIN);
    //     if (lv_obj_get_child_cnt(obj) > 1) {
    //         lv_obj_set_style_pad_top(label, 5, LV_PART_MAIN);
    //         lv_obj_align_to(label, lv_obj_get_child(obj, lv_obj_get_child_cnt(obj) - 2), LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    //     }
    // }
}
