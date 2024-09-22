#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_err.h>
#include <driver/i2c.h>

#define M5_I2C_PORT 0
#define M5_SDA_PIN 21
#define M5_SCL_PIN 22
#define M5_I2C_SPEED_HZ 100000 // 100kHz

#define I2C_TIMEOUT_MS 2000

typedef enum
{
    I2C_REG_ADDR_SIZE_BYTE = 1,
    I2C_REG_ADDR_SIZE_WORD = 2
} i2c_reg_addr_size_t;

esp_err_t i2c_init(void);
esp_err_t i2c_deinit(void);

esp_err_t i2c_read(uint8_t dev_addr, uint16_t reg_addr, i2c_reg_addr_size_t reg_addr_size, void *data, size_t size);
esp_err_t i2c_write(uint8_t dev_addr, uint16_t reg_addr, i2c_reg_addr_size_t reg_addr_size, const void *data, size_t size);

esp_err_t i2c_check_presence(uint8_t dev_addr);

#ifdef __cplusplus
}
#endif
