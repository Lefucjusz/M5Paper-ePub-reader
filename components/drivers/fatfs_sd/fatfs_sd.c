#include "fatfs_sd.h"
#include <spi.h>
#include <esp_vfs_fat.h>

static sdmmc_card_t *card = NULL;

esp_err_t fatfs_sd_init(void) 
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = M5_SPI_HOST;
    host.max_freq_khz = FATFS_SD_SPI_CLOCK_SPEED_KHZ;

    const sdspi_device_config_t dev_cfg = {
        .host_id   = M5_SPI_HOST,
        .gpio_cs   = FATFS_SD_SPI_CS_PIN,
        .gpio_cd   = GPIO_NUM_NC,
        .gpio_wp   = GPIO_NUM_NC,
        .gpio_int  = GPIO_NUM_NC
    };

    const esp_vfs_fat_mount_config_t mnt_cfg = {
        .format_if_mount_failed = false,
        .max_files = FATFS_SD_MAX_FILES_NUM,
        .allocation_unit_size = FATFS_SD_ALLOCATION_UNIT_SIZE,
        .disk_status_check_enable = false
    };

    return esp_vfs_fat_sdspi_mount(FATFS_SD_ROOT_PATH, &host, &dev_cfg, &mnt_cfg, &card);
}

esp_err_t fatfs_sd_deinit(void)
{
    return esp_vfs_fat_sdcard_unmount(FATFS_SD_ROOT_PATH, card);
}
