#include "StatusBar.hpp"
#include "style/Style.hpp"
#include "Colors.hpp"
// #include "gui_set_time_popup.h"
#include "Fonts.h"
#include <real_time_clock.h>
#include <battery.h>
#include <esp_log.h>
#include <string>
#include <cstdint>

#define TAG __FILENAME__

namespace gui
{
    namespace
    {
        lv_obj_t *clockLabel;
        lv_obj_t *batteryIcon;
        lv_obj_t *batteryLabel;

        auto setBatteryIcon(std::uint8_t batteryPercent) -> void
        {
            if (batteryPercent < 20) {
                lv_label_set_text(batteryIcon, LV_SYMBOL_BATTERY_EMPTY);
            }
            else if ((batteryPercent >= 20) && (batteryPercent < 40)) {
                lv_label_set_text(batteryIcon, LV_SYMBOL_BATTERY_1);
            }
            else if ((batteryPercent >= 40) && (batteryPercent < 60)) {
                lv_label_set_text(batteryIcon, LV_SYMBOL_BATTERY_2);
            }
            else if ((batteryPercent >= 60) && (batteryPercent < 90)) {
                lv_label_set_text(batteryIcon, LV_SYMBOL_BATTERY_3);
            }
            else {
                lv_label_set_text(batteryIcon, LV_SYMBOL_BATTERY_FULL);
            }
            lv_obj_align_to(batteryIcon, batteryLabel, LV_ALIGN_OUT_LEFT_MID, style::icon::offsetX, style::icon::offsetY);
        }
    }

    auto clockClickCallback(lv_event_t *event) -> void
    {
        ESP_LOGW(TAG, "TODO: setting time");
    }

    auto statusBarCreate() -> void
    {
        /* Create status bar */
        auto statusBar = lv_obj_create(lv_scr_act());
        lv_obj_set_size(statusBar, style::status_bar::width, style::status_bar::height);
        lv_obj_align(statusBar, LV_ALIGN_TOP_MID, 0, style::margins::horizontalTop);
        lv_obj_set_style_border_side(statusBar, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
        // lv_obj_set_style_border_color(statusBar, GUI_COLOR_LIGHT_GREY, LV_PART_MAIN); // TODO
        lv_obj_set_style_border_width(statusBar, style::border::width, LV_PART_MAIN);
        lv_obj_clear_flag(statusBar, LV_OBJ_FLAG_SCROLLABLE);

        /* Create clock */
        clockLabel = lv_label_create(statusBar);
        lv_obj_set_height(clockLabel, style::status_bar::height);
        lv_obj_center(clockLabel);
        lv_obj_add_flag(clockLabel, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(clockLabel, clockClickCallback, LV_EVENT_CLICKED, nullptr);

        /* Create battery label */
        batteryLabel = lv_label_create(statusBar);
        lv_obj_set_height(batteryLabel, style::status_bar::height);
        lv_obj_set_style_text_font(batteryLabel, &gui_montserrat_medium_20, LV_PART_MAIN);
        lv_obj_align(batteryLabel, LV_ALIGN_RIGHT_MID, style::label::offsetX, style::label::offsetY);

        /* Create battery icon */
        batteryIcon = lv_label_create(statusBar);
        lv_obj_set_height(batteryIcon, style::status_bar::height);
        lv_obj_set_style_text_font(batteryIcon, &gui_montserrat_medium_28, LV_PART_MAIN);

        /* Refresh status bar */
        statusBarUpdate();
    }

    auto statusBarUpdate() -> void
    {
        struct tm time;
        real_time_clock_get_time(&time);

        /* Refresh time */
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d:%02d", time.tm_hour, time.tm_min);
        lv_label_set_text(clockLabel, buffer);

        /* Refresh battery level */
        const auto batteryLevel = battery_get_percent();
        const auto batteryVoltage = battery_get_voltage_filtered();
        snprintf(buffer, sizeof(buffer), "%d%% (%01.2fV)", batteryLevel, batteryVoltage / 1000.0f);
        lv_label_set_text(batteryLabel, buffer);
        setBatteryIcon(batteryLevel);
    }
}
