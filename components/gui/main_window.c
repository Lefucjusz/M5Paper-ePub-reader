#include "main_window.h"
#include "lvgl.h"
#include "real_time_clock.h"
#include "battery.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>
#include <stdio.h>

#include "lvgl_task.h"

#include <esp_wifi.h>
#include "esp_log.h"

static lv_obj_t *clock_label;
static lv_obj_t *battery_symbol;
static lv_obj_t *battery_label;
static lv_timer_t *clock_timer;
static lv_obj_t *main_area;

static void set_battery_symbol(uint8_t percent)
{
    if (percent < 20) {
        lv_label_set_text(battery_symbol, LV_SYMBOL_BATTERY_EMPTY);
    }
    else if (percent >= 20 && percent < 40) {
        lv_label_set_text(battery_symbol, LV_SYMBOL_BATTERY_1);
    }
    else if (percent >= 40 && percent < 60) {
        lv_label_set_text(battery_symbol, LV_SYMBOL_BATTERY_2);
    }
    else if (percent >= 60 && percent < 90) {
        lv_label_set_text(battery_symbol, LV_SYMBOL_BATTERY_3);
    }
    else {
        lv_label_set_text(battery_symbol, LV_SYMBOL_BATTERY_FULL);
    }
}

static void on_clock_update(lv_timer_t *timer)
{
    static int last_h;
    static int last_m;

    struct tm time;
    real_time_clock_get_time(&time);

    if ((last_h != time.tm_hour) || (last_m != time.tm_min)) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", time.tm_hour, time.tm_min);
        lv_label_set_text(clock_label, buf);

        if (time.tm_min == 0 || timer == NULL) {
            const uint8_t battery_percent = battery_get_percent();
            snprintf(buf, sizeof(buf), "%u%%", battery_percent);
            lv_label_set_text(battery_label, buf);
            set_battery_symbol(battery_percent);
        }

        last_h = time.tm_hour;
        last_m = time.tm_min;
    }
}   

static void settings_close_cb(lv_event_t *event)
{
    lv_obj_del(lv_event_get_user_data(event));
}

static void settings_cb(lv_event_t *event)
{
    lv_obj_t *settings_window = lv_win_create(main_area, 60);
    lv_obj_set_style_opa(settings_window, 255, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(settings_window, 255, LV_PART_MAIN);
    lv_obj_set_style_bg_color(settings_window, lv_color_make(170, 170, 170), LV_PART_MAIN);
    lv_win_add_title(settings_window, "Settings");

    lv_obj_t *close_button = lv_win_add_btn(settings_window, LV_SYMBOL_CLOSE, 60);
    lv_obj_add_event_cb(close_button, settings_close_cb, LV_EVENT_CLICKED, settings_window);
}

void main_window_create(void)
{
    const lv_coord_t status_bar_height = 36;
    const lv_coord_t screen_height = 960;
    const lv_coord_t screen_width = 540;
    const lv_coord_t top_offset = 10;
    const lv_coord_t battery_to_clock_offset_x = 138;
    const lv_coord_t battery_symbol_to_label_offset = 8;

    /* Create status bar */
    lv_obj_t *status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, screen_width, status_bar_height);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, top_offset);
    lv_obj_set_style_border_side(status_bar, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    // lv_obj_set_style_bg_color(status_bar, lv_color_make(180, 180, 180), LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(status_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(status_bar, lv_color_make(95, 96, 96), LV_PART_MAIN);
    lv_obj_set_style_border_width(status_bar, 2, LV_PART_MAIN);
    lv_obj_set_style_border_opa(status_bar, LV_OPA_COVER, LV_PART_MAIN);

    /* Add objects to status bar */
    /* Create clock */
    clock_label = lv_label_create(status_bar);
    lv_obj_set_height(clock_label, status_bar_height);
    lv_obj_center(clock_label);
    clock_timer = lv_timer_create(on_clock_update, 1000, NULL);

    /* Create battery symbol */
    battery_symbol = lv_label_create(status_bar);
    lv_obj_set_height(battery_symbol, status_bar_height);
    lv_obj_set_style_text_font(battery_symbol, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_label_set_text(battery_symbol, LV_SYMBOL_BATTERY_3);
    lv_obj_align_to(battery_symbol, clock_label, LV_ALIGN_OUT_RIGHT_MID, battery_to_clock_offset_x, 0);

    /* Create battery label */
    battery_label = lv_label_create(status_bar);
    lv_obj_set_height(battery_label, status_bar_height);
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_align_to(battery_label, battery_symbol, LV_ALIGN_OUT_RIGHT_MID, battery_symbol_to_label_offset, 3);

    on_clock_update(NULL); // Force status bar refresh

    /* Create main area */
    main_area = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_area, screen_width, screen_height - status_bar_height);
    lv_obj_align_to(main_area, status_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    // lv_scr_load(main_area);

    /* Create example icon */
    lv_obj_t *app_button = lv_btn_create(main_area);
    lv_obj_set_size(app_button, 90, 90);
    lv_obj_set_style_radius(app_button, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_color(app_button, lv_color_make(180, 180, 180), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(app_button, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(app_button, LV_ALIGN_OUT_BOTTOM_LEFT, 30, 20);
    lv_obj_add_event_cb(app_button, settings_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *app_button_icon = lv_label_create(app_button);
    lv_obj_set_style_text_font(app_button_icon, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_label_set_text(app_button_icon, LV_SYMBOL_SETTINGS);
    lv_obj_center(app_button_icon);

    lv_obj_t *app_button_label = lv_label_create(main_area);
    lv_obj_set_style_text_font(app_button_label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_label_set_text(app_button_label, "Settings");
    lv_obj_align_to(app_button_label, app_button, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
}
