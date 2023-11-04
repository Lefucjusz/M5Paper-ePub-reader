#include "touch.h"
#include "GT911.h"
#include "utils.h"

struct touch_ctx_t
{
    uint16_t last_x;
    uint16_t last_y;
    touch_rotation_t rotation;
};

static struct touch_ctx_t touch_ctx;

touch_err_t touch_init(touch_rotation_t rotation)
{
    touch_ctx.last_x = 0;
    touch_ctx.last_y = 0;
    touch_ctx.rotation = rotation;

    gpio_config_t gpio_cfg = {
        .intr_type = GPIO_INTR_DISABLE, // TODO for now interrupt pin is unused
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << TOUCH_IRQ_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };

    /* Initialize peripherals */
    if (unlikely(gpio_config(&gpio_cfg) != ESP_OK)) {
        return TOUCH_GPIO_ERROR;
    }
    if (unlikely(i2c_init(TOUCH_I2C_PORT, TOUCH_SDA_PIN, TOUCH_SCL_PIN, TOUCH_I2C_SPEED_HZ) != ESP_OK)) { // TODO this should be moved outside
        return TOUCH_I2C_ERROR;
    }
    return TOUCH_OK;
}

touch_err_t touch_deinit(void)
{
    return TOUCH_OK; // TODO i2c_deinit
}

touch_err_t touch_get_coords(struct touch_coords_t *coords) // TODO add multiple touches handling
{
    /* Sanity check */
    if (unlikely(coords == NULL)) {
        return TOUCH_INVALID_ARG;
    }

    /* Read status register */
    uint8_t status_reg;
    esp_err_t err = i2c_read(GT911_I2C_ADDRESS, GT911_TOUCH_STATUS_REG, GT911_REG_ADDR_SIZE_BYTES, &status_reg, sizeof(status_reg));
    if (unlikely(err != ESP_OK)) {
        return TOUCH_I2C_ERROR;
    }

    /* If data valid and screen touched - read new touch coordinates */
    const bool data_valid = status_reg & GT911_TOUCH_DATA_VALID_MASK;
    const uint8_t touch_count = status_reg & GT911_TOUCH_POINTS_COUNT_MASK;
    if (!data_valid) {
        coords->x = touch_ctx.last_x;
        coords->y = touch_ctx.last_y;
        coords->state = TOUCH_RELEASED;
        return TOUCH_OK;
    }

    if (touch_count > 0) {
        /* Read touch data */
        uint8_t touch_data[GT911_TOUCH_COORDS_SIZE];
        err = i2c_read(GT911_I2C_ADDRESS, GT911_POINT_1_X_COORD_LSB, GT911_REG_ADDR_SIZE_BYTES, &touch_data, sizeof(touch_data));
        if (unlikely(err != ESP_OK)) {
            return TOUCH_I2C_ERROR;
        }
        
        /* Transform coordinates */
        const uint16_t touch_x = MAKE_WORD(touch_data[1], touch_data[0]);
        const uint16_t touch_y = MAKE_WORD(touch_data[3], touch_data[2]);
        switch (touch_ctx.rotation) {
            case TOUCH_ROTATION_0:
                coords->x = touch_y;
                coords->y = TOUCH_PANEL_WIDTH - touch_x;
                break;
            case TOUCH_ROTATION_90:
                coords->x = touch_x;
                coords->y = touch_y;
                break;
            case TOUCH_ROTATION_180:
                coords->x = TOUCH_PANEL_HEIGHT - touch_y;
                coords->y = touch_x;
                break;
            case TOUCH_ROTATION_270:
                coords->x = TOUCH_PANEL_WIDTH - touch_x;
                coords->y = TOUCH_PANEL_HEIGHT - touch_y;
                break;
            default:
                break;
        }
        coords->state = TOUCH_PRESSED;
        touch_ctx.last_x = coords->x;
        touch_ctx.last_y = coords->y;
    } 
    else {
        coords->x = touch_ctx.last_x;
        coords->y = touch_ctx.last_y;
        coords->state = TOUCH_RELEASED;
    }

    /* Clear status register */
    status_reg = 0;
    err = i2c_write(GT911_I2C_ADDRESS, GT911_TOUCH_STATUS_REG, GT911_REG_ADDR_SIZE_BYTES, &status_reg, sizeof(status_reg));
    if (unlikely(err != ESP_OK)) {
        return TOUCH_I2C_ERROR;
    }
    return TOUCH_OK;
}
