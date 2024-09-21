#pragma once

#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t sleep_init(void);
esp_err_t sleep_enter(void);

#ifdef __cplusplus
}
#endif
