#include "spi.h"

esp_err_t spi_init(void)
{
    const spi_bus_config_t spi_cfg = { // TODO this should be moved outside like I2C
        .miso_io_num = M5_SPI_MISO_PIN,
        .mosi_io_num = M5_SPI_MOSI_PIN,
        .sclk_io_num = M5_SPI_SCK_PIN,
        .quadhd_io_num = -1, // Unused
        .quadwp_io_num = -1, // Unused
        .max_transfer_sz = SPI_MAX_TRANSFER_SIZE_BYTES
    };

    return spi_bus_initialize(M5_SPI_HOST, &spi_cfg, SPI_DMA_CH_AUTO);
}

esp_err_t spi_deinit(void)
{
    return spi_bus_free(M5_SPI_HOST);
}
