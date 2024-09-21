#include <lvgl_task.h>
#include <i2c.h>
#include <spi.h>
#include <fatfs_sd.h>
#include <battery.h>
#include <real_time_clock.h>
#include <Gui.hpp>
#include <driver/gpio.h>
#include <esp_log.h>

/* TODO: 
 * - constexprs instead of defines in GUI
 * - move GUI style constants to separate files
 * - put whole GUI into namespace
 * - reimplement remaining GUI elements
 * - implement or delete clock
 * - implement powering off
 * - fix compilation warnings (battery, lvgl)
 * - implement simultaneous rendering and drawing
 * - update I2C driver
 */

extern "C" void app_main()
{
    // gpio_config_t gpio_cfg = {
    //     .intr_type = GPIO_INTR_DISABLE,
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pin_bit_mask = (1 << 2),
    //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
    //     .pull_up_en = GPIO_PULLUP_DISABLE
    // };

    // ESP_ERROR_CHECK(gpio_config(&gpio_cfg));

    // gpio_set_level(2, 0); // Power switch

    ESP_ERROR_CHECK(i2c_init());
    ESP_ERROR_CHECK(spi_init());
    ESP_ERROR_CHECK(real_time_clock_init());
    ESP_ERROR_CHECK(fatfs_sd_init());

    lvgl_task_init();
    gui::create(FATFS_SD_ROOT_PATH);
    lvgl_task_start();

    // TODO perform cleanup somewhere
}
