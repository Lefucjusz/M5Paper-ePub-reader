#include "lvgl_task.h"
#include "lvgl.h"
#include "i2c.h"
#include "spi.h"
#include "fatfs_sd.h"
#include "battery.h"
#include "dir.h"
#include "real_time_clock.h"
#include "gui.h"
#include "gui_page.h"
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

    gpio_set_level(2, 0); // Power switch

    ESP_ERROR_CHECK(i2c_init());
    ESP_ERROR_CHECK(spi_init());
    ESP_ERROR_CHECK(real_time_clock_init());
    ESP_ERROR_CHECK(fatfs_sd_init());

    dir_init(FATFS_SD_ROOT_PATH);

    battery_init();
    lvgl_task_init(); // TODO this should rather be gui_task

    gui_create();
    // gui_page_create(NULL, 0);
   
    lvgl_task_start();

    // TODO perform cleanup somewhere
}
