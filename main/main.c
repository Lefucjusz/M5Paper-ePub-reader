#include "lvgl_task.h"
#include "lvgl.h"
#include "i2c.h"
#include "spi.h"
#include "fatfs_sd.h"
#include "battery.h"
#include "dir.h"
#include "real_time_clock.h"
#include "gui_status_bar.h"
#include "gui_files_list.h"
#include "gui_toc_list.h"
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

    gpio_set_level(2, 1); // Power switch

    ESP_ERROR_CHECK(i2c_init());
    ESP_ERROR_CHECK(spi_init());
    ESP_ERROR_CHECK(real_time_clock_init());
    ESP_ERROR_CHECK(fatfs_sd_init());

    dir_init(FATFS_SD_ROOT_PATH);

    battery_init();
    lvgl_task_init(); // TODO this should rather be gui_task

    // const char *files[] = {
    //     "/sdcard/huxley-limbo.epub",
    //     "/sdcard/Vertical.epub",
    //     "/sdcard/huxley-eyeless-in-gaza.epub",
    //     "/sdcard/huxley-heaven-hell.epub",
    //     "/sdcard/Recepta_na_lepszy_klimat__Zdrowsze_miasta_dla_chorujacego_swiata.epub",
    //     "/sdcard/Chimeryczny lokator - Roland Topor.epub"
    // };

    // for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); ++i) {
    //     epub_err_t err = epub_open(files[i]);
    //     if (!err) {
    //         epub_close();
    //     }
    //     else {
    //         ESP_LOGE("", "Opening test failed");
    //         break;
    //     }
    // }

    // epub_err_t err = epub_open("/sdcard/huxley-limbo.epub");
    // epub_err_t err = epub_open("/sdcard/Vertical.epub");
    // if (!err) {
    //     epub_close();
    // }

    gui_status_bar_create();

    // gui_toc_list_create();

    gui_files_list_create();

    lvgl_task_start();

    // TODO perform cleanup somewhere
}
