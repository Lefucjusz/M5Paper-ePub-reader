#include "lvgl_task.h"
#include "lvgl.h"
#include "i2c.h"
#include "spi.h"
#include "battery.h"
#include "real_time_clock.h"
#include "gui_status_bar.h"
#include <esp_log.h>
#include <driver/gpio.h>

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

    ESP_ERROR_CHECK(i2c_init());
    ESP_ERROR_CHECK(spi_init());

    real_time_clock_init();
    battery_init();
    lvgl_task_init(); // TODO this should rather be gui_task

    gui_status_bar_create();

    lvgl_task_start();
}
