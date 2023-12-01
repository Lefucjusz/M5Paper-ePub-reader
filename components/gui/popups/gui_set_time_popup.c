#include "gui_set_time_popup.h"
#include "lvgl.h"
#include "real_time_clock.h"
#include <esp_log.h>
#include <stdio.h>

#define GUI_SET_TIME_POPUP_TITLE "Set time"

#define GUI_HOURS_PER_DAY 24
#define GUI_MINUTES_PER_HOUR 60

#define GUI_HR_ROLLER_DATA_SIZE (GUI_HOURS_PER_DAY * 3 + 1) // 24 * (2 digits + '\n') + '\0'
#define GUI_MIN_ROLLER_DATA_SIZE (GUI_MINUTES_PER_HOUR * 3 + 1) // 60 * (2 digits + '\n') + '\0'

struct gui_set_time_popup_ctx 
{
    lv_obj_t *msgbox;
    lv_obj_t *hr_roller;
    lv_obj_t *min_roller;
    char *hr_roller_data;
    char *min_roller_data;
};

static struct gui_set_time_popup_ctx ctx;

static bool gui_create_roller_data(void);
static void gui_cancel_callback(lv_event_t *event);
static void gui_save_callback(lv_event_t *event);

void gui_set_time_popup_create(void)
{
    /* Create message box */
    ctx.msgbox = lv_msgbox_create(NULL, GUI_SET_TIME_POPUP_TITLE, "", NULL, false); // TODO empty string
    lv_obj_set_style_border_side(ctx.msgbox, LV_BORDER_SIDE_FULL, LV_PART_MAIN);
    lv_obj_set_style_border_width(ctx.msgbox, 2, LV_PART_MAIN);
    lv_obj_center(ctx.msgbox);

    /* Create roller data */
    gui_create_roller_data();

    /* Create hour roller */
    ctx.hr_roller = lv_roller_create(ctx.msgbox);
    lv_obj_set_style_text_font(ctx.hr_roller, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_roller_set_options(ctx.hr_roller, ctx.hr_roller_data, LV_ROLLER_MODE_NORMAL); // TODO styles
    lv_roller_set_visible_row_count(ctx.hr_roller, 3);
    lv_obj_set_style_border_side(ctx.hr_roller, LV_BORDER_SIDE_FULL, LV_PART_MAIN);
    lv_obj_set_style_border_width(ctx.hr_roller, 2, LV_PART_MAIN);
    lv_obj_align(ctx.hr_roller, LV_ALIGN_CENTER, 50, 0);

    /* Create minute roller */
    ctx.min_roller = lv_roller_create(ctx.msgbox);
    lv_obj_set_style_text_font(ctx.min_roller, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_roller_set_options(ctx.min_roller , ctx.min_roller_data, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(ctx.min_roller, 3);
    lv_obj_set_style_border_side(ctx.min_roller, LV_BORDER_SIDE_FULL, LV_PART_MAIN);
    lv_obj_set_style_border_width(ctx.min_roller, 2, LV_PART_MAIN);
    lv_obj_align(ctx.min_roller, LV_ALIGN_CENTER, -50, 0);

    /* Create buttons */
    lv_obj_t *cancel_btn = lv_btn_create(ctx.msgbox);
    lv_obj_add_event_cb(cancel_btn, gui_cancel_callback, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(cancel_btn, 100, 50);
    lv_obj_set_style_border_side(cancel_btn, LV_BORDER_SIDE_FULL, LV_PART_MAIN);
    lv_obj_set_style_border_width(cancel_btn, 2, LV_PART_MAIN);
    lv_obj_align(cancel_btn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_t *cancel_btn_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_btn_label, "Cancel");
    lv_obj_center(cancel_btn_label);

    lv_obj_t *save_btn = lv_btn_create(ctx.msgbox);
    lv_obj_add_event_cb(save_btn, gui_save_callback, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(save_btn, 100, 50);
    lv_obj_set_style_border_side(save_btn, LV_BORDER_SIDE_FULL, LV_PART_MAIN);
    lv_obj_set_style_border_width(save_btn, 2, LV_PART_MAIN);
    lv_obj_align(save_btn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_t *save_btn_label = lv_label_create(save_btn);
    lv_label_set_text(save_btn_label, "Save");
    lv_obj_center(save_btn_label);

    /* Set roller value based on current time */
    struct tm time;
    real_time_clock_get_time(&time);
    lv_roller_set_selected(ctx.hr_roller, time.tm_hour, LV_ANIM_OFF);
    lv_roller_set_selected(ctx.min_roller, time.tm_min, LV_ANIM_OFF);
}

void gui_set_time_popup_destroy(void)
{
    lv_msgbox_close(ctx.msgbox);
    free(ctx.hr_roller_data);
    free(ctx.min_roller_data);
}

static bool gui_create_roller_data(void)
{
    /* Allocate memory */
    ctx.hr_roller_data = malloc(GUI_HR_ROLLER_DATA_SIZE);
    if (ctx.hr_roller_data == NULL) {
        return false;
    }

    ctx.min_roller_data = malloc(GUI_MIN_ROLLER_DATA_SIZE);
    if (ctx.hr_roller_data == NULL) {
        free(ctx.hr_roller_data);
        return false;
    }

    /* Create hour roller data */
    size_t pos = 0;
    for (uint8_t i = 0; i < GUI_HOURS_PER_DAY; ++i) {
        pos += sprintf(&ctx.hr_roller_data[pos], "%u\n", i);
    }
    ctx.hr_roller_data[pos - 1] = '\0';

    /* Create minute roller data */
    pos = 0;
    for (uint8_t i = 0; i < GUI_MINUTES_PER_HOUR; ++i) {
        pos += sprintf(&ctx.min_roller_data[pos], "%02u\n", i);
    }
    ctx.min_roller_data[pos - 1] = '\0';

    return true;
}

static void gui_cancel_callback(lv_event_t *event)
{
    gui_set_time_popup_destroy();
}

static void gui_save_callback(lv_event_t *event)
{
    /* Get ID instead of string to avoid conversion - it maps 1:1 in this case */
    const uint16_t hr_set = lv_roller_get_selected(ctx.hr_roller);
    const uint16_t min_set = lv_roller_get_selected(ctx.min_roller);

    /* Update time */
    struct tm time;
    real_time_clock_get_time(&time);
    time.tm_hour = hr_set;
    time.tm_min = min_set;
    time.tm_sec = 0;
    real_time_clock_set_time(&time);

    gui_set_time_popup_destroy();
}
