#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#define REAL_TIME_CLOCK_I2C_PORT I2C_NUM_0

#define REAL_TIME_CLOCK_YEARS_PER_CENTURY 100

typedef enum
{
    REAL_TIME_CLOCK_OK,
    REAL_TIME_CLOCK_I2C_ERROR,
    REAL_TIME_CLOCK_INVALID_ARG,
    REAL_TIME_CLOCK_LOW_VOLTAGE,
    REAL_TIME_CLOCK_GENERAL_ERROR
} real_time_clock_err_t;

real_time_clock_err_t real_time_clock_init(void);

real_time_clock_err_t real_time_clock_set_time(const struct tm *time);
real_time_clock_err_t real_time_clock_get_time(struct tm *time);

real_time_clock_err_t real_time_clock_set_alarm(const struct tm *alarm);
real_time_clock_err_t real_time_clock_get_alarm(struct tm *alarm);

#ifdef __cplusplus
}
#endif
