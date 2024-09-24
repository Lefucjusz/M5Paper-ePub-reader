#include <i2c.h>
#include <spi.h>
#include <fatfs_sd.h>
#include <battery.h>
#include <real_time_clock.h>
#include <lvgl_task.h>
#include <Gui.hpp>
#include <driver/gpio.h>
#include <esp_log.h>

/* TODO: 
 * - implement clock setting
 * - implement powering off
 * - add "loading" popup
 */

extern "C" void app_main()
{
    // gpio_config_t gpio_cfg = {
    //     .pin_bit_mask = (1 << 2),
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pull_up_en = GPIO_PULLUP_DISABLE,
    //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
    //     .intr_type = GPIO_INTR_DISABLE
    // };

    // ESP_ERROR_CHECK(gpio_config(&gpio_cfg));

    // gpio_set_level((gpio_num_t)2, 1); // Power switch

    ESP_ERROR_CHECK(i2c_init());
    ESP_ERROR_CHECK(spi_init());
    ESP_ERROR_CHECK(real_time_clock_init());
    ESP_ERROR_CHECK(fatfs_sd_init());

    battery_init();

    // struct tm time;
    // memset(&time, 0, sizeof(time));

    // time.tm_hour = 18;
    // time.tm_min = 8;
    // time.tm_sec = 0;

    // real_time_clock_set_time(&time);

    lvgl_task_init();
    gui::create(FATFS_SD_ROOT_PATH);
    lvgl_task_start();
    
    // TODO perform cleanup somewhere
}
