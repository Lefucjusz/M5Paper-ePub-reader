#pragma once

#include <esp_err.h>
#include <driver/spi_master.h>

#define M5_SPI_HOST HSPI_HOST
#define M5_SPI_MISO_PIN 13
#define M5_SPI_MOSI_PIN 12
#define M5_SPI_SCK_PIN 14

#define SPI_MAX_TRANSFER_SIZE_BYTES 8192

esp_err_t spi_init(void);
esp_err_t spi_deinit(void);
