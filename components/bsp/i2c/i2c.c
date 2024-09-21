#include "i2c.h"
#include <endian.h>

#define I2C_MAKE_ADDR(addr, mode) (((addr) << 1) | (mode))
#define I2C_ACK_CHECK_DISABLE 0
#define I2C_ACK_CHECK_ENABLE 1

esp_err_t i2c_init(void)
{
    const i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = M5_SDA_PIN,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_io_num = M5_SCL_PIN,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = M5_I2C_SPEED_HZ,
        .clk_flags = 0
    };

    /* Configure and install peripheral */
    esp_err_t err = i2c_param_config(M5_I2C_PORT, &i2c_cfg);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(M5_I2C_PORT, i2c_cfg.mode, 0, 0, 0);
}

esp_err_t i2c_deinit(void)
{
    return i2c_driver_delete(M5_I2C_PORT);
}

esp_err_t i2c_read(uint8_t dev_addr, uint16_t reg_addr, i2c_reg_addr_size_t reg_addr_size, void *data, size_t size)
{
    /* Sanity check */
    if ((reg_addr_size != I2C_REG_ADDR_SIZE_BYTE) && (reg_addr_size != I2C_REG_ADDR_SIZE_WORD)) {
        return ESP_ERR_INVALID_ARG;
    }
    if ((size == 0) || (data == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }

    /* Create handle */
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL) {
        return ESP_ERR_NO_MEM;
    }

    /* Fix endianness if register address size is word */
    if (reg_addr_size == I2C_REG_ADDR_SIZE_WORD) {
        reg_addr = __bswap16(reg_addr);
    }

    /* Read data */
    uint8_t *data_ptr = (uint8_t *)data;
    esp_err_t err;
    do {
        /* Start transmission and select register to read */
        err = i2c_master_start(cmd);
        if (err != ESP_OK) {
            break;
        }
        err = i2c_master_write_byte(cmd, I2C_MAKE_ADDR(dev_addr, I2C_MASTER_WRITE), I2C_ACK_CHECK_ENABLE);
        if (err != ESP_OK) {
            break;
        }
        err = i2c_master_write(cmd, (uint8_t *)&reg_addr, reg_addr_size, I2C_ACK_CHECK_ENABLE);
        if (err != ESP_OK) {
            break;
        }

        /* Repeat start and read data */
        err = i2c_master_start(cmd);
        if (err != ESP_OK) {
            break;
        }
        err = i2c_master_write_byte(cmd, I2C_MAKE_ADDR(dev_addr, I2C_MASTER_READ), I2C_ACK_CHECK_ENABLE);
        if (err != ESP_OK) {
            break;
        }
        if (size > 1) {
            err = i2c_master_read(cmd, data_ptr, size - 1, I2C_MASTER_ACK);
            if (err != ESP_OK) {
                break;
            }
        }
        err = i2c_master_read_byte(cmd, &data_ptr[size - 1], I2C_MASTER_NACK);
        if (err != ESP_OK) {
            break;
        }
        err = i2c_master_stop(cmd);
        if (err != ESP_OK) {
            break;
        }
        err = i2c_master_cmd_begin(M5_I2C_PORT, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
        if (err != ESP_OK) {
            break;
        }
    } while (0);

    /* Delete handle */
    i2c_cmd_link_delete(cmd);
    return err;
}

esp_err_t i2c_write(uint8_t dev_addr, uint16_t reg_addr, i2c_reg_addr_size_t reg_addr_size, const void *data, size_t size)
{
    /* Sanity check */
    if ((reg_addr_size != I2C_REG_ADDR_SIZE_BYTE) && (reg_addr_size != I2C_REG_ADDR_SIZE_WORD)) {
        return ESP_ERR_INVALID_ARG;
    }
    if ((size == 0) || (data == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }

    /* Create handle */
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL) {
        return ESP_ERR_NO_MEM;
    }

    /* Fix endianness if register address if 2-byte */
    if (reg_addr_size == I2C_REG_ADDR_SIZE_WORD) {
        reg_addr = __bswap16(reg_addr);
    }

    /* Write data */
    esp_err_t err;
    do {
        /* Start transmission and select register to read */
        err = i2c_master_start(cmd);
        if (err != ESP_OK) {
            break;
        }
        err = i2c_master_write_byte(cmd, I2C_MAKE_ADDR(dev_addr, I2C_MASTER_WRITE), I2C_ACK_CHECK_ENABLE);
        if (err != ESP_OK) {
            break;
        }
        err = i2c_master_write(cmd, (uint8_t *)&reg_addr, reg_addr_size, I2C_ACK_CHECK_ENABLE);
        if (err != ESP_OK) {
            break;
        }

        /* Write data */
        err = i2c_master_write(cmd, data, size, I2C_ACK_CHECK_ENABLE);
        if (err != ESP_OK) {
            break;
        }
        err = i2c_master_stop(cmd);
        if (err != ESP_OK) {
            break;
        }
        err = i2c_master_cmd_begin(M5_I2C_PORT, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
        if (err != ESP_OK) {
            break;
        }
    } while (0);

    /* Delete handle */
    i2c_cmd_link_delete(cmd);
    return err;
}
