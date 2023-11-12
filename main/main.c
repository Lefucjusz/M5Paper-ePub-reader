#include "lvgl_task.h"
#include "lvgl.h"
#include "i2c.h"
#include "real_time_clock.h"
#include <esp_log.h>
#include <driver/gpio.h>

#define M5_I2C_PORT 0
#define M5_SDA_PIN 21
#define M5_SCL_PIN 22
#define M5_I2C_SPEED_HZ 100000 // 100kHz

static lv_obj_t *keyboard;
static lv_obj_t *text_area;
static lv_obj_t *clock_label;

void on_clock_update(lv_timer_t *timer)
{
    static int last_h;
    static int last_m;

    struct tm time;
    real_time_clock_get_time(&time);

    if ((last_h != time.tm_hour) || (last_m != time.tm_min)) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", time.tm_hour, time.tm_min);
        lv_label_set_text(clock_label, buf);

        last_h = time.tm_hour;
        last_m = time.tm_min;
    }
}   

static void text_area_cb(lv_event_t *event)
{
    switch (lv_event_get_code(event)) {
        case LV_EVENT_FOCUSED:
            lv_keyboard_set_textarea(keyboard, text_area);
            lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
            break;
        case LV_EVENT_DEFOCUSED:
            lv_keyboard_set_textarea(keyboard, NULL);
            lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
            break;
        default:
            break;
    }

}

static void del_cb(lv_event_t *event)
{
    lv_obj_del(lv_obj_get_user_data(event));
}

static void set_cb(lv_event_t *event)
{
    lv_obj_t *win = lv_win_create(lv_scr_act(), 30);
    lv_win_add_title(win, "Settings");

    lv_obj_set_style_bg_color(win, lv_color_make(170, 170, 170), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(win, 255, LV_PART_MAIN);

    lv_obj_t *close_button = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 30);
    lv_obj_add_event_cb(close_button, del_cb, LV_EVENT_CLICKED, win);
}

void app_main(void)
{
    gpio_config_t gpio_cfg = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1 << 2),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&gpio_cfg));

    gpio_set_level(2, 0); // Power switch

    if (unlikely(i2c_init(M5_I2C_PORT, M5_SDA_PIN, M5_SCL_PIN, M5_I2C_SPEED_HZ) != ESP_OK)) {
        ESP_LOGE("I2C", "I2C init failed");
    }

    real_time_clock_init();

    lvgl_init();

    // lv_obj_t * list1 = lv_list_create(lv_scr_act());
    // lv_obj_set_size(list1, 540, 960/4);
    // lv_obj_align(list1, LV_ALIGN_TOP_MID, 0, 0);

    // /*Add buttons to the list*/
    // lv_obj_t * list_btn;

    // list_btn = lv_list_add_btn(list1, LV_SYMBOL_FILE, "New item");
    // lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    // list_btn = lv_list_add_btn(list1, LV_SYMBOL_DIRECTORY, "Test item");
    // lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    // list_btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Delete item");
    // lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    // list_btn = lv_list_add_btn(list1, LV_SYMBOL_EDIT, "Some very long text to test");
    // lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    // list_btn = lv_list_add_btn(list1, LV_SYMBOL_SAVE, "Save item");
    // lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    // list_btn = lv_list_add_btn(list1, LV_SYMBOL_BELL, "Notify item");
    // lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    // for (size_t i = 0; i < 36; ++i) {
    //     char buf[32];
    //     snprintf(buf, 32, "Element with callback %u", i+1);
    //     list_btn = lv_list_add_btn(list1, LV_SYMBOL_BATTERY_FULL, buf);
    //     lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    //     // lv_obj_add_event_cb(list_btn, button_cb, LV_EVENT_CLICKED, NULL);
    // }

    clock_label = lv_label_create(lv_scr_act());
    lv_obj_align(clock_label, LV_ALIGN_TOP_MID, 0, 8);
    on_clock_update(NULL);

    lv_timer_create(on_clock_update, 1000, NULL);

    lv_obj_t *bat_label = lv_label_create(lv_scr_act());
    lv_obj_align_to(bat_label, clock_label, LV_ALIGN_OUT_RIGHT_MID, 196, -3);
    lv_obj_set_style_text_font(bat_label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_FULL);

    // lv_obj_t *button = lv_btn_create(lv_scr_act());
    // lv_obj_center(button);
    // lv_obj_set_size(button, 80, 80);
    // // lv_obj_set_style_border_width(button, 2, LV_PART_MAIN);
    // lv_obj_set_style_bg_color(button, lv_color_make(170, 170, 170), LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(button, 255, LV_PART_MAIN);
    // lv_obj_set_style_radius(button, 6, LV_PART_MAIN);
    // lv_obj_add_event_cb(button, set_cb, LV_EVENT_CLICKED, NULL);

    // lv_obj_t *icon_label = lv_label_create(button);
    // // lv_obj_set_size(icon_label, 110, 30);
    // lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_36, LV_PART_MAIN);
    // lv_label_set_text(icon_label, LV_SYMBOL_SETTINGS);
    // lv_obj_center(icon_label);


    // lv_obj_t *button_label = lv_label_create(lv_scr_act());
    // // lv_obj_set_size(button_label, 90, 30);
    // lv_obj_set_style_text_font(button_label, &lv_font_montserrat_20, LV_PART_MAIN);
    // lv_label_set_text(button_label, "Settings");
    // lv_obj_align_to(button_label, button, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);


    keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(keyboard, 540 - 10, 360);

    text_area = lv_textarea_create(lv_scr_act());
    lv_obj_align(text_area, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_add_event_cb(text_area, text_area_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_size(text_area, 540-20, 400);

    lv_obj_set_style_border_color(text_area, lv_color_make(170, 170, 170), LV_PART_MAIN);
    lv_obj_set_style_border_width(text_area, 4, LV_PART_MAIN);
    lv_obj_set_style_radius(text_area, 5, LV_PART_MAIN);

    // lv_obj_set_style_bg_color(text_area, lv_color_make(170, 170, 170), LV_PART_SELECTED);


    // lv_obj_set_style_border_color(text_area, lv_color_make(159, 160, 159), LV_PART_MAIN);
    lv_obj_set_style_border_width(text_area, 2, LV_PART_CURSOR);
    lv_obj_set_style_border_side(text_area, LV_BORDER_SIDE_LEFT, LV_PART_CURSOR);


    // lv_obj_set_style_border_side(lv_textarea_get_label(text_area), LV_BORDER_SIDE_FULL, LV_PART_MAIN);


    // lv_obj_set_style_text_color(text_area, lv_color_make(159, 160, 159), LV_PART_MAIN);
    // lv_obj_set_style_border_width(text_area, 10, LV_PART_CURSOR);
    // lv_textarea_set_cursor_hi
    // lv_obj_set_style_opa(text_area, 0, LV_PART_CURSOR);
    // lv_obj_add_state(text_area, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/

    // lv_obj_t *label = lv_label_create(lv_scr_act());
    // lv_obj_set_style_text_font(label, &lv_font_montserrat_36, LV_PART_MAIN);
    // lv_obj_set_style_text_color(label, lv_color_make(90, 100, 100), LV_PART_MAIN);
    // lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    // lv_label_set_text(label, "Very long text to benchmark scrolling capabilities");
    // lv_obj_set_size(label, 500, 200);
    // lv_obj_center(label);

    // lv_obj_t *label2 = lv_label_create(lv_scr_act());
    // lv_obj_set_style_text_font(label2, &lv_font_montserrat_36, LV_PART_MAIN);
    // lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);
    // lv_label_set_text(label2, "Yet another text enabling to test speed of eink refreshes");
    // lv_obj_set_size(label2, 500, 200);
    // lv_obj_align(label2, LV_ALIGN_CENTER, 0, 75);

    lv_keyboard_set_textarea(keyboard, text_area);

    
    lvgl_task_start();
}
