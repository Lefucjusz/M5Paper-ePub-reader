#include "eink.h"
#include "utils.h"
#include <string.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#define BYTES_TO_BITS(x) (x * 8)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define GET_HIGH_WORD(x) (((x) >> 16) & 0xFFFF)
#define GET_LOW_WORD(x) ((x) & 0xFFFF)
#define MAKE_BYTE(hi,lo) (((hi) << 4) | (lo))
#define MAKE_WORD(hi,lo) (((hi) << 8) | (lo))

/* TODO:
 * - deinit;
 * - entering sleep mode after refresh;
 * - better transfer handling - no polling;
 * - doxygen;
 */

struct eink_ctx_t
{
    spi_device_handle_t spi;
    eink_rotation_t rotation;
    eink_color_t color;
    SemaphoreHandle_t hrdy_semaphore;
    uint8_t *spi_buffer; // 32-bit aligned DMA accessible buffer used for SPI transfer
};

static struct eink_ctx_t eink_ctx;

/* Private functions forward declarations */
static esp_err_t spi_transfer(const void *tx_buffer, void *rx_buffer, size_t size);
static esp_err_t spi_read16(uint16_t *data);
static esp_err_t spi_write16(uint16_t data);

static esp_err_t eink_gpio_config(void);
static esp_err_t eink_spi_config(void);

static void eink_hrdy_isr(void *arg);
static eink_err_t eink_wait_hrdy(void);
static eink_err_t eink_wait_afsr(void);

static eink_err_t eink_read_word(uint16_t *data);
static eink_err_t eink_write_word(uint16_t data);

static eink_err_t eink_write_command(uint16_t command);
static eink_err_t eink_read_register(uint16_t address, uint16_t *data);
static eink_err_t eink_write_register(uint16_t address, uint16_t data);
static eink_err_t eink_write_args(uint16_t command, const uint16_t *args, size_t length);

static eink_err_t eink_set_target_memory_address(uint32_t address);
static eink_err_t eink_set_area(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/* Public functtions */
eink_err_t eink_init(eink_rotation_t rotation, eink_color_t color)
{
    eink_ctx.rotation = rotation;
    eink_ctx.color = color;

    /* Create HRDY IRQ semaphore */
    eink_ctx.hrdy_semaphore = xSemaphoreCreateBinary();
    if (eink_ctx.hrdy_semaphore == NULL) {
        return EINK_NO_MEMORY;
    }

    /* Allocate SPI buffer */
    eink_ctx.spi_buffer = heap_caps_malloc(EINK_SPI_MAX_TRANSFER_SIZE_BYTES, MALLOC_CAP_32BIT | MALLOC_CAP_DMA);
    if (eink_ctx.spi_buffer == NULL) {
        vSemaphoreDelete(eink_ctx.hrdy_semaphore);
        return EINK_NO_MEMORY;
    }
    
    /* Initialize peripherals */
    if (eink_gpio_config() != ESP_OK) {
        return EINK_GPIO_ERROR;
    }
    if (eink_spi_config() != ESP_OK) {
        return EINK_SPI_ERROR;
    }

    /* Turn on the controller, enable I80 pack write */
    eink_err_t err = eink_write_command(IT8951_TCON_SYS_RUN);
    if (err) {
        return err;
    }
    err = eink_write_register(IT8951_I80CPCR, 0x0001);
    if (err) {
        return err;
    }

    /* Set VCOM to -2.3V */
    err = eink_write_command(IT8951_I80_CMD_VCOM);
    if (err) {
        return err;
    }
    err = eink_write_word(0x0001); // Write operation
    if (err) {
        return err;
    }
    err = eink_write_word(2300); // -2300mV
    if (err) {
        return err;
    }

    /* Set internal controller RAM target memory address */
    err = eink_set_target_memory_address(EINK_TARGET_MEMORY_ADDRESS);
    if (err) {
        return err;
    }

    /* Clear the display */
    err = eink_clear();
    if (err) {
        return err;
    }

    /* Initialize the display */
    return eink_refresh_full(EINK_UPDATE_MODE_INIT);
}

void eink_deinit(void)
{
    // TODO deinit SPI, turn off display power, delete semaphore, free spi_buffer
}

void eink_set_rotation(eink_rotation_t rotation)
{
    eink_ctx.rotation = rotation;
}

eink_rotation_t eink_get_rotation(void)
{
    return eink_ctx.rotation;
}

void eink_set_color(eink_color_t color)
{
    eink_ctx.color = color;
}

eink_rotation_t eink_get_color(void)
{
    return eink_ctx.color;
}

eink_err_t eink_clear(void)
{
    /* Set target memory address */
    eink_err_t err = eink_set_target_memory_address(EINK_TARGET_MEMORY_ADDRESS);
    if (err) {
        return err;
    }

    /* Set area */
    switch (eink_ctx.rotation) {
        case EINK_ROTATION_0:
        case EINK_ROTATION_180:
            err = eink_set_area(0, 0, EINK_DISPLAY_WIDTH, EINK_DISPLAY_HEIGHT);
            break;
        case EINK_ROTATION_90:
        case EINK_ROTATION_270:
            err = eink_set_area(0, 0, EINK_DISPLAY_HEIGHT, EINK_DISPLAY_WIDTH);
            break;
        default:
            err = EINK_INVALID_ARG;
            break;
    }
    if (err) {
        return err;
    }

    /* Fill buffer with initial screen color */
    const uint8_t px_value = (eink_ctx.color == EINK_COLOR_NORMAL) ? MAKE_BYTE(EINK_PIXEL_WHITE, EINK_PIXEL_WHITE) : MAKE_BYTE(EINK_PIXEL_BLACK, EINK_PIXEL_BLACK);
    memset(eink_ctx.spi_buffer, px_value, EINK_SPI_MAX_TRANSFER_SIZE_BYTES);

    /* Write data */
    size_t bytes_left = (EINK_DISPLAY_WIDTH * EINK_DISPLAY_HEIGHT) / EINK_PIXELS_PER_BYTE;
    while (bytes_left > 0) {
        const size_t transfer_size = min(bytes_left, EINK_SPI_MAX_TRANSFER_SIZE_BYTES);

        gpio_set_level(EINK_SPI_CS_PIN, EINK_LOW);
        const uint16_t preamble = IT8951_SPI_WRITE_DATA_PREAMBLE;
        if (spi_transfer(&preamble, NULL, sizeof(preamble)) != ESP_OK) { // Send preamble
            return EINK_SPI_ERROR;
        }
        if (spi_transfer(eink_ctx.spi_buffer, NULL, transfer_size) != ESP_OK) { // Send image data
            return EINK_SPI_ERROR;
        }
        gpio_set_level(EINK_SPI_CS_PIN, EINK_HIGH);

        bytes_left -= transfer_size;
    }

    /* End image loading */
    return eink_write_command(IT8951_TCON_LD_IMG_END);
}

eink_err_t eink_write(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *px_map)
{
    /* Sanity check */
    if (px_map == NULL) {
        return EINK_INVALID_ARG;
    }

    /* Display requires 4 pixel horizontal alignment */
    if (((x % 4) != 0) || ((w % 4) != 0)) {
        ESP_LOGE("", "NOT ALIGNED!");
        return EINK_NOT_ALIGNED;
    }

    /* Set target memory address */
    eink_err_t err = eink_set_target_memory_address(EINK_TARGET_MEMORY_ADDRESS);
    if (err) {
        return err;
    }

    /* Set area */
    err = eink_set_area(x, y, w, h);
    if (err) {
        return err;
    }

    /* Send image data */
    const size_t image_data_size = (w * h) / EINK_PIXELS_PER_BYTE;
    size_t bytes_left = image_data_size;
    while (bytes_left > 0) {
        /* Copy data to SPI buffer */
        const size_t transfer_size = min(bytes_left, EINK_SPI_MAX_TRANSFER_SIZE_BYTES);
        memcpy(eink_ctx.spi_buffer, &px_map[image_data_size - bytes_left], transfer_size);

        /* Invert colors if required */
        if (eink_ctx.color == EINK_COLOR_INVERTED) {
            for (size_t i = 0; i < transfer_size; ++i) {
                eink_ctx.spi_buffer[i] = ~eink_ctx.spi_buffer[i];
            }
        }

        /* Write data */
        gpio_set_level(EINK_SPI_CS_PIN, EINK_LOW);
        const uint16_t preamble = IT8951_SPI_WRITE_DATA_PREAMBLE;
        if (spi_transfer(&preamble, NULL, sizeof(preamble)) != ESP_OK) { // Send preamble
            return EINK_SPI_ERROR;
        }
        if (spi_transfer(eink_ctx.spi_buffer, NULL, transfer_size) != ESP_OK) { // Send image data
            return EINK_SPI_ERROR;
        }
        gpio_set_level(EINK_SPI_CS_PIN, EINK_HIGH);

        bytes_left -= transfer_size;
    }

    /* End image loading */
    return eink_write_command(IT8951_TCON_LD_IMG_END);
}

eink_err_t eink_write_full(const uint8_t *px_map)
{
    switch (eink_ctx.rotation) {
        case EINK_ROTATION_0:
        case EINK_ROTATION_180:
            return eink_write(0, 0, EINK_DISPLAY_WIDTH, EINK_DISPLAY_HEIGHT, px_map);
        case EINK_ROTATION_90:
        case EINK_ROTATION_270:
            return eink_write(0, 0, EINK_DISPLAY_HEIGHT, EINK_DISPLAY_WIDTH, px_map);
        default:
            return EINK_INVALID_ARG;
    }
}

eink_err_t eink_refresh(uint16_t x, uint16_t y, uint16_t w, uint16_t h, eink_update_mode_t mode)
{
    if (mode == EINK_UPDATE_MODE_NONE) {
        return EINK_INVALID_ARG;
    }

    /* Display requires 4 pixel horizontal alignment */
    if (((x % 4) != 0) || ((w % 4) != 0)) {
        return EINK_NOT_ALIGNED;
    }

    uint16_t args[7];

    /* Transform coordinates */
    switch (eink_ctx.rotation) {
        case EINK_ROTATION_0:
            args[0] = x;
            args[1] = y;
            args[2] = w;
            args[3] = h;
            break;
        case EINK_ROTATION_90:
            args[0] = y;
            args[1] = EINK_DISPLAY_HEIGHT - (w + x);
            args[2] = h;
            args[3] = w;
            break;
        case EINK_ROTATION_180:
            args[0] = EINK_DISPLAY_WIDTH - (w + x);
            args[1] = EINK_DISPLAY_HEIGHT - (h + y);
            args[2] = w;
            args[3] = h;
            break;
        case EINK_ROTATION_270:
            args[0] = EINK_DISPLAY_WIDTH - (h + y);
            args[1] = x;
            args[2] = h;
            args[3] = w;
            break;
    }

    args[4] = mode;
    args[5] = GET_LOW_WORD(EINK_TARGET_MEMORY_ADDRESS);
    args[6] = GET_HIGH_WORD(EINK_TARGET_MEMORY_ADDRESS);

    /* Write to display */
    const eink_err_t err = eink_write_args(IT8951_I80_CMD_DPY_BUF_AREA, args, ARRAY_SIZE(args));
    if (err) {
        return err;
    }

    /* Wait for LUT engine to finish operation */
    return eink_wait_afsr();
}

eink_err_t eink_refresh_full(eink_update_mode_t mode)
{
    switch (eink_ctx.rotation) {
        case EINK_ROTATION_0:
        case EINK_ROTATION_180:
            return eink_refresh(0, 0, EINK_DISPLAY_WIDTH, EINK_DISPLAY_HEIGHT, mode);
        case EINK_ROTATION_90:
        case EINK_ROTATION_270:
            return eink_refresh(0, 0, EINK_DISPLAY_HEIGHT, EINK_DISPLAY_WIDTH, mode);
        default:
            return EINK_INVALID_ARG;
    }
}


/* Private functions */
static esp_err_t spi_transfer(const void *tx_buffer, void *rx_buffer, size_t size)
{
    spi_transaction_t transaction = {
        .tx_buffer = tx_buffer,
        .rx_buffer = rx_buffer,
        .length = BYTES_TO_BITS(size),
        .rxlength = BYTES_TO_BITS(size)
    };

    return spi_device_polling_transmit(eink_ctx.spi, &transaction);
}

static esp_err_t spi_read16(uint16_t *data)
{
    const esp_err_t err = spi_transfer(NULL, data, sizeof(*data));
    *data = bswap16(*data); // ESP32 is little endian, IT8951 big endian
    return err;
}

static esp_err_t spi_write16(uint16_t data)
{
    data = bswap16(data);
    return spi_transfer(&data, NULL, sizeof(data));
}

static esp_err_t eink_gpio_config(void)
{
    /* Configure HRDY pin */
    gpio_config_t gpio_cfg = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1 << EINK_SPI_HRDY_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };

    esp_err_t err = gpio_config(&gpio_cfg);
    if (err) {
        return err;
    }
    err = gpio_install_isr_service(0);
    if (err) {
        return err;
    }
    err = gpio_isr_handler_add(EINK_SPI_HRDY_PIN, eink_hrdy_isr, NULL);
    if (err) {
        return err;
    }

    /* Configure CS pin */
    gpio_cfg.intr_type = GPIO_INTR_DISABLE;
    gpio_cfg.mode = GPIO_MODE_OUTPUT;
    gpio_cfg.pin_bit_mask = (1 << EINK_SPI_CS_PIN);
    gpio_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    err = gpio_config(&gpio_cfg);
    if (err) {
        return err;
    }
    gpio_set_level(EINK_SPI_CS_PIN, EINK_HIGH);

    /* Configure EPD power enable pin and turn the power on */
    gpio_cfg.pin_bit_mask = (1 << EINK_PWR_EN_PIN);
    err = gpio_config(&gpio_cfg);
    if (err) {
        return err;
    }
    gpio_set_level(EINK_PWR_EN_PIN, EINK_HIGH);

    return ESP_OK;
}

static esp_err_t eink_spi_config(void)
{
    const spi_bus_config_t spi_cfg = {
        .miso_io_num = EINK_SPI_MISO_PIN,
        .mosi_io_num = EINK_SPI_MOSI_PIN,
        .sclk_io_num = EINK_SPI_SCK_PIN,
        .quadhd_io_num = -1, // Unused
        .quadwp_io_num = -1, // Unused
        .max_transfer_sz = EINK_SPI_MAX_TRANSFER_SIZE_BYTES
    };
    
    const spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = EINK_SPI_CLOCK_SPEED_HZ,
        .mode = EINK_SPI_MODE,
        .spics_io_num = -1, // Manual CS
        .queue_size = 1
    };

    const esp_err_t err = spi_bus_initialize(EINK_SPI_HOST, &spi_cfg, SPI_DMA_CH_AUTO);
    if (err) {
        return err;
    }
    
    return spi_bus_add_device(EINK_SPI_HOST, &dev_cfg, &eink_ctx.spi);
}

static void IRAM_ATTR eink_hrdy_isr(void *arg)
{
    BaseType_t should_yield;
    xSemaphoreGiveFromISR(eink_ctx.hrdy_semaphore, &should_yield);
    portYIELD_FROM_ISR(should_yield);
}

static eink_err_t eink_wait_hrdy(void)
{
    /* If already done - just return */
    if (gpio_get_level(EINK_SPI_HRDY_PIN) == EINK_HIGH) {
        return EINK_OK;
    }
    
    if (xSemaphoreTake(eink_ctx.hrdy_semaphore, pdMS_TO_TICKS(EINK_TIMEOUT_MS)) == pdFALSE) {
        return EINK_TIMEOUT;
    }
    return EINK_OK;
}

static eink_err_t eink_wait_afsr(void)
{
    const TickType_t init_ticks = xTaskGetTickCount();
    while (1) {
        uint16_t reg_value;
        const eink_err_t err = eink_read_register(IT8951_LUTAFSR, &reg_value);

        /* Readout failure */
        if (err) {
            return err;
        }

        /* LUT engine idle */
        if (reg_value == 0) {
            return EINK_OK;
        }

        /* Timeout */
        if ((xTaskGetTickCount() - init_ticks) >= pdMS_TO_TICKS(EINK_TIMEOUT_MS)) {
            return EINK_TIMEOUT;
        }
        
        /* Poll with predefined interval */
        vTaskDelay(pdMS_TO_TICKS(EINK_AFSR_POLLING_INTERVAL_MS));
    }
}

static eink_err_t eink_read_word(uint16_t *data)
{
    eink_err_t err = EINK_OK;
    do {
        err = eink_wait_hrdy();
        if (err) {
            break;
        }
        gpio_set_level(EINK_SPI_CS_PIN, EINK_LOW);
        if (spi_write16(IT8951_SPI_READ_DATA_PREAMBLE) != ESP_OK) {
            err = EINK_SPI_ERROR;
            break;
        }
        err = eink_wait_hrdy();
        if (err) {
            break;
        }
        if (spi_write16(0x0000) != ESP_OK) { // Discard dummy data
            err = EINK_SPI_ERROR;
            break;
        }
        err = eink_wait_hrdy();
        if (err) {
            break;
        }
        if (spi_read16(data) != ESP_OK) {
            err = EINK_SPI_ERROR;
            break;
        }
    } while (0);
    
    gpio_set_level(EINK_SPI_CS_PIN, EINK_HIGH);
    return err;
}

static eink_err_t eink_write_word(uint16_t data)
{
    eink_err_t err = EINK_OK;
    do {
        err = eink_wait_hrdy();
        if (err) {
            break;
        }
        gpio_set_level(EINK_SPI_CS_PIN, EINK_LOW);
        if (spi_write16(IT8951_SPI_WRITE_DATA_PREAMBLE) != ESP_OK) {
            err = EINK_SPI_ERROR;
            break;
        }
        err = eink_wait_hrdy();
        if (err) {
            break;
        }
        if (spi_write16(data) != ESP_OK) {
            err = EINK_SPI_ERROR;
            break;
        }
    } while (0);

    gpio_set_level(EINK_SPI_CS_PIN, EINK_HIGH);
    return err;
}

static eink_err_t eink_write_command(uint16_t command) 
{
    eink_err_t err = EINK_OK;
    do {
        err = eink_wait_hrdy();
        if (err) {
            break;
        }
        gpio_set_level(EINK_SPI_CS_PIN, EINK_LOW);
        if (spi_write16(IT8951_SPI_CMD_PREAMBLE) != ESP_OK) {
            err = EINK_SPI_ERROR;
            break;
        }
        err = eink_wait_hrdy();
        if (err) {
            break;
        }
        if (spi_write16(command) != ESP_OK) {
            err = EINK_SPI_ERROR;
            break;
        }
    } while (0);

    gpio_set_level(EINK_SPI_CS_PIN, EINK_HIGH);
    return err;
}

static eink_err_t eink_read_register(uint16_t address, uint16_t *data)
{
    eink_err_t err = eink_write_command(IT8951_TCON_REG_RD);
    if (err) {
        return err;
    }
    err = eink_write_word(address);
    if (err) {
        return err;
    } 
    return eink_read_word(data);
}

static eink_err_t eink_write_register(uint16_t address, uint16_t data)
{
    eink_err_t err = eink_write_command(IT8951_TCON_REG_WR);
    if (err) {
        return err;
    }
    err = eink_write_word(address);
    if (err) {
        return err;
    }
    return eink_write_word(data);
}

static eink_err_t eink_write_args(uint16_t command, const uint16_t *args, size_t length)
{
    eink_err_t err = eink_write_command(command);
    if (err) {
        return err;
    }

    for (size_t i = 0; i < length; ++i) {
        err = eink_write_word(args[i]);
        if (err) {
            return err;
        }
    }

    return EINK_OK;
}

static eink_err_t eink_set_target_memory_address(uint32_t address)
{
    eink_err_t err = eink_write_register(IT8951_LISAR + 2, GET_HIGH_WORD(address));
    if (err) {
        return err;
    }
    return eink_write_register(IT8951_LISAR, GET_LOW_WORD(address));
}

static eink_err_t eink_set_area(uint16_t x, uint16_t y, uint16_t w, uint16_t h) 
{
    /* Fixed big endian 4bpp format */
    const uint16_t flags = (IT8951_LDIMG_B_ENDIAN << IT8951_ENDIANNESS_SHIFT) |
                           (IT8951_4BPP << IT8951_PIXEL_MODE_SHIFT) |
                           (eink_ctx.rotation << IT8951_ROTATION_SHIFT); 
    const uint16_t args[] = {flags, x, y, w, h};
    
    /* Set area */
    return eink_write_args(IT8951_TCON_LD_IMG_AREA, args, ARRAY_SIZE(args));
}
