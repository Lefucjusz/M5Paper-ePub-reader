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

void close_cb(lv_event_t *event)
{
    lv_obj_del(lv_event_get_user_data(event));
}

void app_cb(lv_event_t *event)
{
    lv_obj_t *win = lv_win_create(lv_scr_act(), 50);
    lv_win_add_title(win, "App");

    lv_obj_t *close_button = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 50);
    lv_obj_add_event_cb(close_button, close_cb, LV_EVENT_CLICKED, win);

    lv_obj_t *cont = lv_win_get_content(win);
    lv_obj_t *label = lv_label_create(cont);

    lv_label_set_text(label, "This is some dummy content to check whether the window will show at all...");
    lv_obj_set_size(label, 500, 960 - 50);
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

    real_time_clock_err_t err = real_time_clock_init();
    ESP_LOGE("", "Error: %d", err);

    struct tm time;
    while (1) {
        ESP_ERROR_CHECK(real_time_clock_get_time(&time));
        ESP_LOGI("", "%s", asctime(&time));
        vTaskDelay(50);
    }

    

    // lvgl_init();

    // lv_obj_t *button = lv_btn_create(lv_scr_act());
    // lv_obj_align(button, LV_ALIGN_CENTER, -40, -90);
    // lv_obj_add_event_cb(button, app_cb, LV_EVENT_CLICKED, NULL);

    // lv_obj_t *button_label = lv_label_create(button);
    // lv_label_set_text(button_label, "App");
    // lv_obj_center(button_label);

    // lvgl_task_start();
}
