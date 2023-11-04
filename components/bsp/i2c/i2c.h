#pragma once

#include <esp_err.h>
#include <driver/i2c.h>

#define I2C_TIMEOUT_MS 2000

typedef enum
{
    I2C_REG_ADDR_SIZE_BYTE = 1,
    I2C_REG_ADDR_SIZE_WORD = 2
} i2c_reg_addr_size_t;

esp_err_t i2c_init(i2c_port_t port, int sda_pin, int scl_pin, uint32_t scl_speed);
esp_err_t i2c_deinit(void);

esp_err_t i2c_read(uint8_t dev_addr, uint16_t reg_addr, i2c_reg_addr_size_t reg_addr_size, void *data, size_t size);
esp_err_t i2c_write(uint8_t dev_addr, uint16_t reg_addr, i2c_reg_addr_size_t reg_addr_size, const void *data, size_t size);
