#include "touch_panel.h"
#include "GT911.h"
#include <utils.h>
#include <esp_log.h>

#define TAG __FILENAME__

typedef struct
{
    uint16_t last_x;
    uint16_t last_y;
    touch_panel_rotation_t rotation;
    uint8_t i2c_address;
} touch_panel_ctx_t;

static touch_panel_ctx_t ctx;

touch_panel_err_t touch_panel_init(touch_panel_rotation_t rotation)
{
    ctx.last_x = 0;
    ctx.last_y = 0;
    ctx.rotation = rotation;

    const gpio_config_t gpio_cfg = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << TOUCH_PANEL_IRQ_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };

    /* Initialize peripherals */
    if (gpio_config(&gpio_cfg) != ESP_OK) {
        return TOUCH_PANEL_GPIO_ERROR;
    }

    /* Workaround for hardware bug - GT911 address depends
     * on its INT pin state during hardware reset. If the pin 
     * is low, the address will be 0x5D, when it's high, the
     * address will be 0x14. INT pin is directly connected to
     * ESP, which seems to change its state during booting.
     * In M5Paper GT911 hardware reset is done either by 
     * UART control lines or by power button, each happens 
     * with INT pin with different state, hence different I2C 
     * address is configured. That's why both have to be 
     * checked here. */
    const uint8_t i2c_addresses[] = {GT911_I2C_ADDRESS_1, GT911_I2C_ADDRESS_2};
    for (size_t i = 0; i < ARRAY_SIZE(i2c_addresses); ++i) {
        ESP_LOGI(TAG, "Trying I2C address 0x%X", i2c_addresses[i]);
        if (i2c_check_presence(i2c_addresses[i]) != ESP_OK) {
            ESP_LOGI(TAG, "Address not found...");
        }
        else {
            ESP_LOGI(TAG, "Address OK");
            ctx.i2c_address = i2c_addresses[i];
            return TOUCH_PANEL_OK;
        }
    }
    ESP_LOGE(TAG, "Touch panel controller not found!");
    return TOUCH_PANEL_I2C_ERROR;
}

touch_panel_err_t touch_get_coords(touch_panel_coords_t *coords) // TODO add multiple touches handling
{
    /* Sanity check */
    if (coords == NULL) {
        return TOUCH_PANEL_INVALID_ARG;
    }

    /* Read status register */
    uint8_t status_reg;
    esp_err_t err = i2c_read(ctx.i2c_address, GT911_TOUCH_STATUS_REG, GT911_REG_ADDR_SIZE_BYTES, &status_reg, sizeof(status_reg));
    if (err != ESP_OK) {
        return TOUCH_PANEL_I2C_ERROR;
    }

    /* If data valid and screen touched - read new touch coordinates */
    const bool data_valid = status_reg & GT911_TOUCH_DATA_VALID_MASK;
    const uint8_t touch_count = status_reg & GT911_TOUCH_POINTS_COUNT_MASK;
    if (!data_valid) {
        coords->x = ctx.last_x;
        coords->y = ctx.last_y;
        coords->state = TOUCH_PANEL_RELEASED;
        return TOUCH_PANEL_OK;
    }

    if (touch_count > 0) {
        /* Read touch data */
        uint8_t touch_data[GT911_TOUCH_COORDS_SIZE];
        err = i2c_read(ctx.i2c_address, GT911_POINT_1_X_COORD_LSB_REG, GT911_REG_ADDR_SIZE_BYTES, &touch_data, sizeof(touch_data));
        if (err != ESP_OK) {
            return TOUCH_PANEL_I2C_ERROR;
        }
        
        /* Transform coordinates */
        const uint16_t touch_x = MAKE_WORD(touch_data[1], touch_data[0]);
        const uint16_t touch_y = MAKE_WORD(touch_data[3], touch_data[2]);
        switch (ctx.rotation) {
            case TOUCH_PANEL_ROTATION_0:
                coords->x = touch_y;
                coords->y = TOUCH_PANEL_WIDTH - touch_x;
                break;
            case TOUCH_PANEL_ROTATION_90:
                coords->x = touch_x;
                coords->y = touch_y;
                break;
            case TOUCH_PANEL_ROTATION_180:
                coords->x = TOUCH_PANEL_HEIGHT - touch_y;
                coords->y = touch_x;
                break;
            case TOUCH_PANEL_ROTATION_270:
                coords->x = TOUCH_PANEL_WIDTH - touch_x;
                coords->y = TOUCH_PANEL_HEIGHT - touch_y;
                break;
            default:
                break;
        }
        coords->state = TOUCH_PANEL_PRESSED;
        ctx.last_x = coords->x;
        ctx.last_y = coords->y;
    } 
    else {
        coords->x = ctx.last_x;
        coords->y = ctx.last_y;
        coords->state = TOUCH_PANEL_RELEASED;
    }

    /* Clear status register */
    status_reg = 0;
    err = i2c_write(ctx.i2c_address, GT911_TOUCH_STATUS_REG, GT911_REG_ADDR_SIZE_BYTES, &status_reg, sizeof(status_reg));
    if (err != ESP_OK) {
        return TOUCH_PANEL_I2C_ERROR;
    }
    return TOUCH_PANEL_OK;
}
