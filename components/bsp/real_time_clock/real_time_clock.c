#include "real_time_clock.h"
#include "BM8563.h"
#include <i2c.h>
#include <utils.h>

real_time_clock_err_t real_time_clock_init(void)
{
    const uint8_t reg_value = 0;

    /* Disable POR, disable STOP, enable normal mode */
    if (i2c_write(BM8563_I2C_ADDRESS, BM8563_CONTROL_STATUS_1_REG, BM8563_REG_ADDR_SIZE_BYTES, &reg_value, sizeof(reg_value)) != ESP_OK) {
        return REAL_TIME_CLOCK_I2C_ERROR;
    }

    /* Disable timer and alarm interrupt, clear interrupt flags */
    if (i2c_write(BM8563_I2C_ADDRESS, BM8563_CONTROL_STATUS_2_REG, BM8563_REG_ADDR_SIZE_BYTES, &reg_value, sizeof(reg_value)) != ESP_OK) {
        return REAL_TIME_CLOCK_I2C_ERROR;
    }

    return REAL_TIME_CLOCK_OK;
}

real_time_clock_err_t real_time_clock_set_time(const struct tm *time)
{
    /* Sanity check */
    if (time == NULL) {
        return REAL_TIME_CLOCK_INVALID_ARG;
    }

    uint8_t time_buffer[BM8563_TIME_DATA_SIZE];

    /* Fill RTC data buffer */
    time_buffer[0] = dec2bcd(time->tm_sec) & BM8563_SECONDS_MASK;
    time_buffer[1] = dec2bcd(time->tm_min) & BM8563_MINUTES_MASK;
    time_buffer[2] = dec2bcd(time->tm_hour) & BM8563_HOURS_MASK;
    time_buffer[3] = dec2bcd(time->tm_mday) & BM8563_DAYS_MASK;
    time_buffer[4] = dec2bcd(time->tm_wday) & BM8563_WEEKDAYS_MASK;
    time_buffer[5] = dec2bcd(time->tm_mon + 1) & BM8563_MONTHS_MASK;
    if (time->tm_year >= REAL_TIME_CLOCK_YEARS_PER_CENTURY) {
        time_buffer[5] |= BM8563_CENTURY_MASK;
    }
    time_buffer[6] = dec2bcd(time->tm_year % REAL_TIME_CLOCK_YEARS_PER_CENTURY);

    if (i2c_write(BM8563_I2C_ADDRESS, BM8563_VL_SECONDS_REG, BM8563_REG_ADDR_SIZE_BYTES, time_buffer, sizeof(time_buffer)) != ESP_OK) {
        return REAL_TIME_CLOCK_I2C_ERROR;
    }

    return REAL_TIME_CLOCK_OK;
}

real_time_clock_err_t real_time_clock_get_time(struct tm *time)
{
    /* Sanity check */
    if (time == NULL) {
        return REAL_TIME_CLOCK_INVALID_ARG;
    }

    uint8_t time_buffer[BM8563_TIME_DATA_SIZE];

    /* Read entire time data packet */
    if (i2c_read(BM8563_I2C_ADDRESS, BM8563_VL_SECONDS_REG, BM8563_REG_ADDR_SIZE_BYTES, time_buffer, sizeof(time_buffer)) != ESP_OK) {
        return REAL_TIME_CLOCK_I2C_ERROR;
    }

    /* Fill time struct  */
    time->tm_sec = bcd2dec(time_buffer[0] & BM8563_SECONDS_MASK);
    time->tm_min = bcd2dec(time_buffer[1] & BM8563_MINUTES_MASK);
    time->tm_hour = bcd2dec(time_buffer[2] & BM8563_HOURS_MASK);
    time->tm_mday = bcd2dec(time_buffer[3] & BM8563_DAYS_MASK);
    time->tm_wday = bcd2dec(time_buffer[4] & BM8563_WEEKDAYS_MASK);
    time->tm_mon = bcd2dec(time_buffer[5] & BM8563_MONTHS_MASK) - 1;
    time->tm_year = bcd2dec(time_buffer[6]);
    if (time_buffer[5] & BM8563_CENTURY_MASK) {
        time->tm_year += REAL_TIME_CLOCK_YEARS_PER_CENTURY;
    }

    /* Compute tm_yday */
    mktime(time);

    /* Check valid bit */
    if (time_buffer[0] & BM8563_VL_MASK) {
        return REAL_TIME_CLOCK_LOW_VOLTAGE;
    }

    return REAL_TIME_CLOCK_OK;
}

real_time_clock_err_t real_time_clock_set_alarm(const struct tm *alarm)
{
    return REAL_TIME_CLOCK_GENERAL_ERROR;
}

real_time_clock_err_t real_time_clock_get_alarm(struct tm *alarm)
{
    return REAL_TIME_CLOCK_GENERAL_ERROR;
}
