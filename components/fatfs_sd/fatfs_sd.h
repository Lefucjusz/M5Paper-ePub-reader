#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_err.h>

#define FATFS_SD_SPI_CS_PIN 4
#define FATFS_SD_SPI_CLOCK_SPEED_KHZ 1000 // 1MHz

#define FATFS_SD_ROOT_PATH "/sdcard"
#define FATFS_SD_MAX_FILES_NUM 2
#define FATFS_SD_ALLOCATION_UNIT_SIZE 0 // Use sector size

esp_err_t fatfs_sd_init(void);
esp_err_t fatfs_sd_deinit(void);

#ifdef __cplusplus
}
#endif
