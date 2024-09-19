#include "lvgl_task.h"
#include "i2c.h"
#include "spi.h"
#include "fatfs_sd.h"
#include "battery.h"
#include "real_time_clock.h"
// #include "gui.h"
#include <driver/gpio.h>
#include <esp_log.h>

#include "FilesList.hpp"

#include "Epub.hpp"

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

    // dir_init(FATFS_SD_ROOT_PATH);

    // try {
    //     auto epub = std::make_unique<Epub>("/sdcard/huxley-heaven-hell.epub");
    //     auto section = epub->getSection("OEBPS/ch05.xhtml");

    //     // ESP_LOGI("", "%s", section.getRaw().c_str());

    //     auto blocks = section.getBlocks();

    //     for (auto &b : blocks) {
    //         ESP_LOGI("", "%s\n", b.text.c_str());
    //     }
    // } catch (const std::exception &e) {
    //     ESP_LOGE("", "Exception while opening epub: %s", e.what());
    // }

    // battery_init();
    lvgl_task_init(); // TODO this should rather be gui_task

    gui::filesListCreate("/sdcard");

    // gui_create();
   
    lvgl_task_start();

    // TODO perform cleanup somewhere
}
