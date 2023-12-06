#include "miniz_test.h"
#include "../third_party/miniz/miniz.h" // TODO fix including file from ESP-IDF
#include <esp_log.h>
#include <fcntl.h>
#include "lvgl.h"

void miniz_test(void)
{
    mz_zip_archive zip;
    mz_zip_zero_struct(&zip);

    if (!mz_zip_reader_init_file(&zip, "/sdcard/Vertical.epub", 0)) {
        ESP_LOGE("", "Failed to open EPUB file");
        return;
    }

    ESP_LOGI("", "Starting extraction");

    uint32_t start = lv_tick_get();
    mkdir("/sdcard/unpacked", 0755);
    mz_zip_reader_extract_file_to_file(&zip, "test/part0006.html", "/sdcard/unpacked/part0006.html", 0);
    uint32_t end = lv_tick_get();

    ESP_LOGI("", "Extraction done in %lums", end - start);

    mz_zip_reader_end(&zip);
}
