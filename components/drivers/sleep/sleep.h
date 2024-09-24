#pragma once

#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t sleep_gpio_init(void);
esp_err_t sleep_timer_init(uint64_t time_us);
esp_err_t sleep_enter(void);

#ifdef __cplusplus
}
#endif
