#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_err.h>
#include <driver/spi_master.h>

#define M5_SPI_HOST HSPI_HOST
#define M5_SPI_MISO_PIN 13
#define M5_SPI_MOSI_PIN 12
#define M5_SPI_SCK_PIN 14

#define SPI_MAX_TRANSFER_SIZE_BYTES 8192

esp_err_t spi_init(void);
esp_err_t spi_deinit(void);

esp_err_t spi_transfer(spi_device_handle_t spi_dev, const void *tx_buffer, void *rx_buffer, size_t size);

#ifdef __cplusplus
}
#endif
