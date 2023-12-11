#include "lvgl_task.h"
#include "lvgl.h"
#include "i2c.h"
#include "spi.h"
#include "fatfs_sd.h"
#include "battery.h"
#include "real_time_clock.h"
#include "gui_status_bar.h"
#include "gui_files_list.h"
#include <esp_log.h>
#include <driver/gpio.h>

#include "epub.h"

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

    battery_init();
    lvgl_task_init(); // TODO this should rather be gui_task

    epub_open("/sdcard/Vertical.epub");
    epub_close();

    gui_status_bar_create();
    gui_files_list_create();

    lvgl_task_start();

    // TODO perform cleanup somewhere
}
