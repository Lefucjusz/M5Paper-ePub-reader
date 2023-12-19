#include "gui_set_time_popup.h"
#include "gui_colors.h"
#include "gui_fonts.h"
#include "real_time_clock.h"
#include <stdio.h>

#define GUI_SET_TIME_POPUP_TITLE (LV_SYMBOL_SETTINGS" Set time")

#define GUI_HOURS_PER_DAY 24
#define GUI_MINUTES_PER_HOUR 60

#define GUI_HR_ROLLER_DATA_SIZE (GUI_HOURS_PER_DAY * 3 + 1) // 24 * (2 digits + '\n') + '\0'
#define GUI_MIN_ROLLER_DATA_SIZE (GUI_MINUTES_PER_HOUR * 3 + 1) // 60 * (2 digits + '\n') + '\0'

typedef struct 
{
    lv_obj_t *msgbox;
    lv_obj_t *hr_roller;
    lv_obj_t *min_roller;
    char *hr_roller_data;
    char *min_roller_data;
} gui_set_time_popup_ctx;

static gui_set_time_popup_ctx ctx;
static const char *button_labels[] = {"Cancel", "Save", NULL};

static bool gui_create_roller_data(void);
static void gui_buttons_callback(lv_event_t *event);

void gui_set_time_popup_create(void)
{
    /* Create message box */
    ctx.msgbox = lv_msgbox_create(NULL, GUI_SET_TIME_POPUP_TITLE, " ", button_labels, false);
    lv_obj_add_event_cb(ctx.msgbox, gui_buttons_callback, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_font(lv_msgbox_get_title(ctx.msgbox), &gui_montserrat_medium_36, LV_PART_MAIN);
    lv_obj_set_style_bg_color(lv_msgbox_get_title(ctx.msgbox), GUI_COLOR_LIGHT_GREY, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_msgbox_get_title(ctx.msgbox), LV_OPA_COVER, LV_PART_MAIN); // TODO fix spacing
    lv_obj_set_width(ctx.msgbox, GUI_SET_TIME_POPUP_WIDTH);
    lv_obj_center(ctx.msgbox);

    /* Create roller data */
    gui_create_roller_data();

    /* Create hour roller */
    ctx.hr_roller = lv_roller_create(lv_msgbox_get_content(ctx.msgbox)); // TODO remove elastic scrolling
    lv_obj_set_style_text_font(ctx.hr_roller, &gui_montserrat_medium_36, LV_PART_MAIN);
    lv_roller_set_options(ctx.hr_roller, ctx.hr_roller_data, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(ctx.hr_roller, 2);
    lv_obj_align_to(ctx.hr_roller, lv_msgbox_get_text(ctx.msgbox), LV_ALIGN_OUT_BOTTOM_MID, -GUI_SET_TIME_POPUP_ROLLER_OFFSET_X, 0);

    /* Create colon */
    lv_obj_t *colon = lv_label_create(lv_msgbox_get_content(ctx.msgbox));
    lv_obj_set_style_text_font(colon, &gui_montserrat_medium_36, LV_PART_MAIN);
    lv_label_set_text(colon, ":");
    lv_obj_align_to(colon, lv_msgbox_get_text(ctx.msgbox), LV_ALIGN_OUT_BOTTOM_MID, 0, GUI_SET_TIME_POPUP_COLON_OFFSET_Y);

    /* Create minute roller */
    ctx.min_roller = lv_roller_create(lv_msgbox_get_content(ctx.msgbox)); // TODO remove elastic scrolling
    lv_obj_set_style_text_font(ctx.min_roller, &gui_montserrat_medium_36, LV_PART_MAIN);
    lv_roller_set_options(ctx.min_roller, ctx.min_roller_data, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(ctx.min_roller, 2);
    lv_obj_clear_flag(ctx.min_roller, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_align_to(ctx.min_roller, lv_msgbox_get_text(ctx.msgbox), LV_ALIGN_OUT_BOTTOM_MID, GUI_SET_TIME_POPUP_ROLLER_OFFSET_X, 0);

    /* Style buttons */
    lv_obj_set_flex_align(ctx.msgbox, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START); // Required for the buttons to be centered
    lv_obj_set_style_bg_color(lv_msgbox_get_btns(ctx.msgbox), GUI_COLOR_LIGHT_GREY, LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(lv_msgbox_get_btns(ctx.msgbox), LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_pad_column(lv_msgbox_get_btns(ctx.msgbox), GUI_SET_TIME_POPUP_BTNS_COL_PAD, LV_PART_MAIN);
    lv_obj_set_style_pad_top(lv_msgbox_get_btns(ctx.msgbox), GUI_SET_TIME_POPUP_BTNS_TOP_PAD, LV_PART_MAIN);
    lv_obj_set_height(lv_msgbox_get_btns(ctx.msgbox), GUI_SET_TIME_POPUP_BTNS_HEIGHT);

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

static void gui_buttons_callback(lv_event_t *event)
{
    lv_obj_t *obj = lv_event_get_current_target(event);

    /* Save */
    if (lv_msgbox_get_active_btn(obj) == 1) {
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
    }

    gui_set_time_popup_destroy();
}
