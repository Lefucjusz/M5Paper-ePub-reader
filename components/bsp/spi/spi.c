#include "spi.h"
#include <utils.h>

esp_err_t spi_init(void)
{
    const spi_bus_config_t spi_cfg = {
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

esp_err_t spi_transfer(spi_device_handle_t spi_dev, const void *tx_buffer, void *rx_buffer, size_t size)
{
    spi_transaction_t transaction = {
        .tx_buffer = tx_buffer,
        .rx_buffer = rx_buffer,
        .length = BYTES_TO_BITS(size),
        .rxlength = BYTES_TO_BITS(size)
    };

    return spi_device_polling_transmit(spi_dev, &transaction);
}
