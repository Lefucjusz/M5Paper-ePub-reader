#pragma once

#include "IT8951.h"
#include <esp_err.h>

#define EINK_DISPLAY_WIDTH 960
#define EINK_DISPLAY_HEIGHT 540

#define EINK_SPI_HOST HSPI_HOST
#define EINK_SPI_CLOCK_SPEED_HZ (24 * 1000 * 1000) // 24MHz - maximum available IT8951 SCLK speed
#define EINK_SPI_MODE 0
#define EINK_SPI_MAX_TRANSFER_SIZE_BYTES 2048

#define EINK_SPI_MISO_PIN 13
#define EINK_SPI_MOSI_PIN 12
#define EINK_SPI_SCK_PIN 14
#define EINK_SPI_CS_PIN 15
#define EINK_SPI_HRDY_PIN 27

#define EINK_PWR_EN_PIN 23

#define EINK_TARGET_MEMORY_ADDRESS 0x001236E0

#define EINK_LOW 0
#define EINK_HIGH 1

#define EINK_TIMEOUT_MS 2000
#define EINK_AFSR_POLLING_INTERVAL_MS 50

#define EINK_PIXELS_PER_BYTE 2
#define EINK_PIXELS_PER_WORD (2 * EINK_PIXELS_PER_BYTE)
#define EINK_WIDTH_PIXELS_ALIGNMENT 4
#define EINK_PIXEL_BLACK 0x00
#define EINK_PIXEL_WHITE 0x0F

typedef enum
{
    EINK_UPDATE_MODE_INIT = 0,
    EINK_UPDATE_MODE_DU,
    EINK_UPDATE_MODE_GC16,
    EINK_UPDATE_MODE_GL16,
    EINK_UPDATE_MODE_GLR16,
    EINK_UPDATE_MODE_GLD16,
    EINK_UPDATE_MODE_DU4,
    EINK_UPDATE_MODE_A2,
    EINK_UPDATE_MODE_NONE 
} eink_update_mode_t;

typedef enum 
{
    EINK_ROTATION_0 = IT8951_ROTATION_0,
    EINK_ROTATION_90 = IT8951_ROTATION_90,
    EINK_ROTATION_180 = IT8951_ROTATION_180,
    EINK_ROTATION_270 = IT8951_ROTATION_270
} eink_rotation_t;

typedef enum
{
    EINK_COLOR_NORMAL,
    EINK_COLOR_INVERTED
} eink_color_t;

typedef enum
{
    EINK_OK,
    EINK_TIMEOUT,
    EINK_OUT_OF_BOUNDS,
    EINK_NOT_ALIGNED,
    EINK_INVALID_ARG,
    EINK_SPI_ERROR,
    EINK_GPIO_ERROR,
    EINK_NO_MEMORY,
    EINK_GENERAL_ERROR
} eink_err_t;


eink_err_t eink_init(eink_rotation_t rotation, eink_color_t color);
void eink_deinit(void);

void eink_set_rotation(eink_rotation_t rotation);
eink_rotation_t eink_get_rotation(void);

void eink_set_color(eink_color_t color);
eink_rotation_t eink_get_color(void);

eink_err_t eink_clear(void);

eink_err_t eink_write(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *px_map);
eink_err_t eink_write_full(const uint8_t *px_map);

eink_err_t eink_refresh(uint16_t x, uint16_t y, uint16_t w, uint16_t h, eink_update_mode_t mode);
eink_err_t eink_refresh_full(eink_update_mode_t mode);
