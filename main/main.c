#include "lvgl_task.h"
#include "lvgl.h"
#include "i2c.h"
#include "battery.h"
#include "real_time_clock.h"
#include "gui_status_bar.h"
#include <esp_log.h>
#include <driver/gpio.h>

/* TODO:
 * - status bar;
 * - sleep mode;
 * - doxygen;
 */

#define M5_I2C_PORT 0
#define M5_SDA_PIN 21
#define M5_SCL_PIN 22
#define M5_I2C_SPEED_HZ 100000 // 100kHz

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

    gpio_set_level(2, 1); // Power switch

    if (unlikely(i2c_init(M5_I2C_PORT, M5_SDA_PIN, M5_SCL_PIN, M5_I2C_SPEED_HZ) != ESP_OK)) {
        ESP_LOGE("I2C", "I2C init failed");
    }

    real_time_clock_init();
    battery_init();
    lvgl_task_init(); // TODO this should rather be gui_task

    gui_status_bar_create();

    lvgl_task_start();
}
