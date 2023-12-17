#include "gui_status_bar.h"
#include "gui_dimensions.h"
#include "gui_colors.h"
#include "gui_set_time_popup.h"
#include "lvgl.h" 
#include "real_time_clock.h"
#include "battery.h"
#include <time.h>
#include <esp_log.h>

struct gui_status_bar_ctx_t
{
    lv_obj_t *clock_label;
    lv_obj_t *battery_icon;
    lv_obj_t *battery_label;
    lv_timer_t *update_timer;
};

static struct gui_status_bar_ctx_t ctx;

/* Private function prototypes */
static void gui_set_battery_icon(uint8_t percent);
static void gui_status_bar_update_callback(lv_timer_t *timer);
static void gui_clock_clicked_event_callback(lv_event_t *event);

/* Public functions */
void gui_status_bar_create(void)
{
    /* Create status bar */
    lv_obj_t *status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, GUI_DISPLAY_WIDTH, GUI_STATUS_BAR_HEIGHT);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, GUI_HORIZONTAL_MARGIN_TOP);
    lv_obj_set_style_border_side(status_bar, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_border_color(status_bar, GUI_COLOR_LIGHT_GREY, LV_PART_MAIN);
    lv_obj_set_style_border_width(status_bar, GUI_STATUS_BAR_BORDER_WIDTH, LV_PART_MAIN);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);

    /* Create clock */
    ctx.clock_label = lv_label_create(status_bar);
    lv_obj_set_height(ctx.clock_label, GUI_STATUS_BAR_HEIGHT);
    lv_obj_center(ctx.clock_label);
    lv_obj_add_flag(ctx.clock_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ctx.clock_label, gui_clock_clicked_event_callback, LV_EVENT_CLICKED, NULL);

    /* Create battery label */
    ctx.battery_label = lv_label_create(status_bar);
    lv_obj_set_height(ctx.battery_label, GUI_STATUS_BAR_HEIGHT);
    lv_obj_set_style_text_font(ctx.battery_label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_align(ctx.battery_label, LV_ALIGN_RIGHT_MID, GUI_BAT_LABEL_OFFSET_X, GUI_BAT_LABEL_OFFSET_Y);

    /* Create battery icon */
    ctx.battery_icon = lv_label_create(status_bar);
    lv_obj_set_height(ctx.battery_icon, GUI_STATUS_BAR_HEIGHT);
    lv_obj_set_style_text_font(ctx.battery_icon, &lv_font_montserrat_28, LV_PART_MAIN);

    /* Create 1s update timer */
    ctx.update_timer = lv_timer_create(gui_status_bar_update_callback, 1000, NULL);

    /* Update status bar */
    gui_status_bar_update_callback(NULL);
}

/* Private function definitions */
static void gui_status_bar_update_callback(lv_timer_t *timer)
{
    static int last_h = -1;
    static int last_m = -1;

    struct tm time;
    real_time_clock_get_time(&time);

    /* Nothing to refresh */
    if ((last_h == time.tm_hour) && (last_m == time.tm_min)) {
        return;
    }

    /* Refresh data */
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d:%02d", time.tm_hour, time.tm_min);
    lv_label_set_text(ctx.clock_label, buffer);

    const uint8_t battery_percent = battery_get_percent();
    snprintf(buffer, sizeof(buffer), "%u%%", battery_percent);
    lv_label_set_text(ctx.battery_label, buffer);
    gui_set_battery_icon(battery_percent);
    
    last_h = time.tm_hour;
    last_m = time.tm_min;
}  

static void gui_set_battery_icon(uint8_t percent)
{
    if (percent < 20) {
        lv_label_set_text(ctx.battery_icon, LV_SYMBOL_BATTERY_EMPTY);
    }
    else if (percent >= 20 && percent < 40) {
        lv_label_set_text(ctx.battery_icon, LV_SYMBOL_BATTERY_1);
    }
    else if (percent >= 40 && percent < 60) {
        lv_label_set_text(ctx.battery_icon, LV_SYMBOL_BATTERY_2);
    }
    else if (percent >= 60 && percent < 90) {
        lv_label_set_text(ctx.battery_icon, LV_SYMBOL_BATTERY_3);
    }
    else {
        lv_label_set_text(ctx.battery_icon, LV_SYMBOL_BATTERY_FULL);
    }
    lv_obj_align_to(ctx.battery_icon, ctx.battery_label, LV_ALIGN_OUT_LEFT_MID, GUI_BAT_ICON_OFFSET_X, GUI_BAT_ICON_OFFSET_Y);
}

static void gui_clock_clicked_event_callback(lv_event_t *event)
{
    gui_set_time_popup_create();
}
