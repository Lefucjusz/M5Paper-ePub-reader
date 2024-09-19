#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c.h"
#include <stdint.h>
#include <esp_err.h>

#define TOUCH_PANEL_WIDTH 540
#define TOUCH_PANEL_HEIGHT 960

#define TOUCH_PANEL_I2C_PORT I2C_NUM_0
#define TOUCH_PANEL_IRQ_PIN 36

#define TOUCH_PANEL_MAX_CONCURRENT_POINTS 2 // GT911 should allow for 5, but predefined M5Paper config enables only 2

typedef enum 
{
    TOUCH_PANEL_ROTATION_0, 
    TOUCH_PANEL_ROTATION_90,
    TOUCH_PANEL_ROTATION_180,
    TOUCH_PANEL_ROTATION_270
} touch_panel_rotation_t;

typedef enum
{
    TOUCH_PANEL_RELEASED,
    TOUCH_PANEL_PRESSED
} touch_panel_state_t;

typedef enum
{
    TOUCH_PANEL_OK,
    TOUCH_PANEL_I2C_ERROR,
    TOUCH_PANEL_INVALID_ARG,
    TOUCH_PANEL_GPIO_ERROR,
    TOUCH_PANEL_NO_MEMORY,
    TOUCH_PANEL_GENERAL_ERROR
} touch_panel_err_t;

typedef struct
{
    uint16_t x;
    uint16_t y;
    touch_panel_state_t state;
} touch_panel_coords_t;

touch_panel_err_t touch_panel_init(touch_panel_rotation_t rotation);
touch_panel_err_t touch_get_coords(touch_panel_coords_t *coords);

#ifdef __cplusplus
}
#endif