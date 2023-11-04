#pragma once

#include "i2c.h"
#include <stdint.h>
#include <esp_err.h>

#define TOUCH_PANEL_WIDTH 540
#define TOUCH_PANEL_HEIGHT 960

#define TOUCH_I2C_PORT I2C_NUM_0
#define TOUCH_I2C_SPEED_HZ 100000 // 100kHz
#define TOUCH_SDA_PIN 21
#define TOUCH_SCL_PIN 22
#define TOUCH_IRQ_PIN 36

#define TOUCH_MAX_CONCURRENT_POINTS 2 // GT911 should allow for 5, but predefined M5Paper config enables only 2

typedef enum 
{
    TOUCH_ROTATION_0, 
    TOUCH_ROTATION_90,
    TOUCH_ROTATION_180,
    TOUCH_ROTATION_270
} touch_rotation_t;

typedef enum
{
    TOUCH_RELEASED,
    TOUCH_PRESSED
} touch_state_t;

typedef enum
{
    TOUCH_OK,
    TOUCH_I2C_ERROR,
    TOUCH_INVALID_ARG,
    TOUCH_GPIO_ERROR,
    TOUCH_NO_MEMORY,
    TOUCH_GENERAL_ERROR
} touch_err_t;

struct touch_coords_t
{
    uint16_t x;
    uint16_t y;
    touch_state_t state;
};

touch_err_t touch_init(touch_rotation_t rotation);
touch_err_t touch_deinit(void);

touch_err_t touch_get_coords(struct touch_coords_t *coords);
