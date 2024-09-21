#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <eink.h>
#include <stdbool.h>

#define LVGL_TASK_STACK_SIZE 16384 // bytes
#define LVGL_TASK_CORE_AFFINITY 1
#define LVGL_TASK_NAME "lvgl_task"

#define LVGL_TICK_TIMER_PERIOD_MS 1
#define LVGL_TICK_TIMER_NAME "lvgl_tick_timer"

#define LVGL_SLEEP_INACTIVITY_PERIOD_MS 50000

#define LVGL_TASK_HANDLER_PERIOD_MS 10

#define LVGL_DRAW_BUFFER_SIZE ((EINK_DISPLAY_WIDTH * EINK_DISPLAY_HEIGHT) / 10) // 1/10 of the whole screen
#define LVGL_FAST_PER_DEEP_REFRESHES 20 // Number of fast refreshes between two deep refreshes

void lvgl_task_init(void);
void lvgl_task_start(void);

#ifdef __cplusplus
}
#endif
