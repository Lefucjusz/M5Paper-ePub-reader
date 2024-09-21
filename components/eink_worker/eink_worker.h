#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <eink.h>
#include <lvgl.h>

#define EINK_WORKER_STACK_SIZE (1024 * 2) // bytes
#define EINK_WORKER_CORE_AFFINITY 1
#define EINK_WORKER_NAME "eink_worker"

#define EINK_WORKER_OPERATION_QUEUE_LENGTH 2 // Max. two operations can be simultaneously queued - write and refresh
#define EINK_WORKER_FAST_PER_DEEP_REFRESHES 12 // Number of fast refreshes between two deep refreshes

eink_err_t eink_worker_start(void (*on_ready)(void));
// void eink_worker_stop(); // TODO

void eink_worker_write(size_t x, size_t y, size_t w, size_t h, lv_color_t *px_map);
void eink_worker_refresh(void);

bool eink_worker_idle(void);

#ifdef __cplusplus
}
#endif
