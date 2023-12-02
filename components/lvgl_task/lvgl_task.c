#include "lvgl_task.h"
#include "utils.h"
#include "touch_panel.h"
#include <lvgl.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

struct lvgl_ctx_t
{
    lv_disp_draw_buf_t disp_buf;
	lv_color_t draw_buf[LVGL_DRAW_BUFFER_SIZE];
	lv_disp_drv_t disp_drv;
	lv_indev_drv_t indev_drv;
    SemaphoreHandle_t lvgl_task_semaphore;
    eink_update_mode_t refresh_mode;
    uint8_t fast_refresh_count;
    TimerHandle_t eink_sleep_timer;
    bool is_eink_awake;
};

static struct lvgl_ctx_t EXT_RAM_BSS_ATTR lvgl_ctx;

/* Private functions declarations */
static void lvgl_transform_pixel_map(lv_color_t *px_map, size_t size);
static void lvgl_refresh_screen(void);
static void lvgl_on_display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *px_map);
static void lvgl_coords_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area);
static void lvgl_on_input_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

static eink_err_t lvgl_display_init(void);
static touch_panel_err_t lvgl_touch_init(void);
static esp_err_t lvgl_tick_timer_init(void);
static esp_err_t lvgl_eink_sleep_timer_init(void);

static eink_err_t lvgl_eink_wakeup(void);

static void lvgl_task(void *arg);
static void lvgl_tick_timer_callback(void *arg);
static void lvgl_eink_sleep_timer_callback(TimerHandle_t timer);

/* Public functions */
void lvgl_task_init(void)
{
    /* Initialize LVGL */
    lv_init();

    /* Create LVGL task semaphore */
    lvgl_ctx.lvgl_task_semaphore = xSemaphoreCreateMutex();

    /* Initialize hardware */
    lvgl_display_init(); // TODO error handling
    lvgl_touch_init();
    lvgl_tick_timer_init();

    lvgl_eink_sleep_timer_init();
}

void lvgl_task_start(void)
{
    xTaskCreatePinnedToCore(lvgl_task, LVGL_TASK_NAME, LVGL_TASK_STACK_SIZE / sizeof(StackType_t), NULL, 0, NULL, LVGL_TASK_CORE_AFFINITY);
}

bool lvgl_task_acquire(void)
{
    return xSemaphoreTake(lvgl_ctx.lvgl_task_semaphore, portMAX_DELAY);
}

void lvgl_task_release(void)
{
    xSemaphoreGive(lvgl_ctx.lvgl_task_semaphore);
}

/* Private functions */
static void lvgl_reset_refresh_mode(void)
{
    /* Default refresh mode */
    lvgl_ctx.refresh_mode = EINK_UPDATE_MODE_A2;
}

static void lvgl_update_refresh_mode(uint8_t px_brightness)
{
    /* The most capable refresh mode already required, return */
    if (lvgl_ctx.refresh_mode == EINK_UPDATE_MODE_GC16) {
        return;
    }

    /* If not handled by A2 */
    if ((px_brightness != EINK_PIXEL_BLACK) && (px_brightness != EINK_PIXEL_WHITE)) {
        if ((px_brightness == EINK_PIXEL_DARK_GRAY_DU4) || (px_brightness == EINK_PIXEL_LIGHT_GRAY_DU4)) {
            lvgl_ctx.refresh_mode = EINK_UPDATE_MODE_DU4; // Can be handled by DU4
        }
        else {
            ESP_LOGW("", "%d", px_brightness);
            lvgl_ctx.refresh_mode = EINK_UPDATE_MODE_GC16; // Can be handled only by GC16
        }
    }
}

static uint8_t lvgl_compute_brightness(lv_color_t color)
{
    /* These coefficients were derived by combining two equations needed to compute pixel brightness:
     * - conversion from RGB332 to RGB888 (max. R value seems to be 7 in LVGL's "RGB232" mode);
     * - conversion from RGB888 to luminance using ITU BT.601 Y = 0.299*R + 0.587*G + 0.114*B.
     * 
     * LVGL's lv_color_brightness() function does it the same way (using slightly different luminance 
     * equation), but splits that computation into two steps. By merging coefficients used in those 
     * steps the same result can be obtained faster, as less operations need to be done. 
     * 
     * Maximum luminance value obtained using this method is equal to:
     * Ymax = (2^3 - 1)*11 + (2^3 - 1)*21 + (2^2 - 1)*10 = 254 */

    const uint8_t red_factor = 11;
    const uint8_t green_factor = 21;
    const uint8_t blue_factor = 10;
    return ((color.ch.red * red_factor) + (color.ch.green * green_factor) + (color.ch.blue * blue_factor));
}

static void lvgl_transform_pixel_map(lv_color_t *px_map, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        const size_t byte_index = i / EINK_PIXELS_PER_BYTE;
        const uint8_t px_index = i % EINK_PIXELS_PER_BYTE;
        const uint8_t px_brightness = map(lvgl_compute_brightness(px_map[i]), 0, 254, EINK_PIXEL_BLACK, EINK_PIXEL_WHITE);
        
        if (px_index) {
            px_map[byte_index].full &= ~EINK_PIXEL_WHITE;
            px_map[byte_index].full |= px_brightness;
        }
        else {
            px_map[byte_index].full &= ~(EINK_PIXEL_WHITE << 4);
            px_map[byte_index].full |= (px_brightness << 4);
        }

        lvgl_update_refresh_mode(px_brightness);
    }
}

static void lvgl_refresh_screen(void)
{
    if ((lvgl_ctx.refresh_mode == EINK_UPDATE_MODE_GC16) || (lvgl_ctx.fast_refresh_count >= LVGL_FAST_PER_DEEP_REFRESHES)) {
        ESP_LOGI("", "Refreshing with GC16");
        eink_refresh_full(EINK_UPDATE_MODE_GC16);
        lvgl_ctx.fast_refresh_count = 0;
    }
    else {
        ESP_LOGI("", "Refreshing with %s", lvgl_ctx.refresh_mode == EINK_UPDATE_MODE_DU4 ? "DU4" : "A2");
        eink_refresh_full(lvgl_ctx.refresh_mode);
        ++lvgl_ctx.fast_refresh_count;
    }
}

static void lvgl_on_display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *px_map)
{
    static uint32_t last_refresh;

    const size_t width = (area->x2 - area->x1) + 1;
	const size_t height = (area->y2 - area->y1) + 1;
    const size_t area_size = width * height;

    lvgl_transform_pixel_map(px_map, area_size);

    lvgl_eink_wakeup();
    eink_write(area->x1, area->y1, width, height, (uint8_t *)px_map);

    if (lv_disp_flush_is_last(disp_drv)) {
        lvgl_refresh_screen();
        lvgl_reset_refresh_mode();
        const uint32_t current_refresh = lv_tick_get();
        ESP_LOGE("PROFILING", "Time between refreshes: %lums", current_refresh - last_refresh);
        last_refresh = current_refresh;
    }

    lv_disp_flush_ready(&lvgl_ctx.disp_drv);
}

static void lvgl_coords_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    /* Align x1 and width to be multiple of 4 */
    area->x1 &= ~(EINK_WIDTH_PIXELS_ALIGNMENT - 1);
    area->x2 |= (EINK_WIDTH_PIXELS_ALIGNMENT - 1);
}

static void lvgl_on_input_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    struct touch_panel_coords_t coords;
    touch_get_coords(&coords);

    data->point.x = coords.x;
    data->point.y = coords.y;
    data->state = coords.state;
}

static eink_err_t lvgl_display_init(void)
{
    /* Set initial state */
    lvgl_ctx.fast_refresh_count = 0;
    lvgl_ctx.is_eink_awake = true;
    lvgl_reset_refresh_mode();

    /* Initialize eink hardware */
    const eink_err_t err = eink_init(EINK_ROTATION_90, EINK_COLOR_NORMAL);
    if (unlikely(err != EINK_OK)) {
       return err;
    }

    /* Initialize display buffer */
    lv_disp_draw_buf_init(&lvgl_ctx.disp_buf, &lvgl_ctx.draw_buf, NULL, LVGL_DRAW_BUFFER_SIZE);

    /* Create display driver */
    lv_disp_drv_init(&lvgl_ctx.disp_drv);
    lvgl_ctx.disp_drv.draw_buf = &lvgl_ctx.disp_buf;
    lvgl_ctx.disp_drv.flush_cb = lvgl_on_display_flush;
    lvgl_ctx.disp_drv.rounder_cb = lvgl_coords_rounder;
	lvgl_ctx.disp_drv.hor_res = EINK_DISPLAY_HEIGHT;
	lvgl_ctx.disp_drv.ver_res = EINK_DISPLAY_WIDTH;

    /* Register display driver */
    lv_disp_t *disp = lv_disp_drv_register(&lvgl_ctx.disp_drv);

    /* Initialize default theme */
    if (lv_theme_mono_is_inited() == false) {
        disp->theme = lv_theme_mono_init(disp, false, LV_FONT_DEFAULT);
    }

    return EINK_OK;
}

static touch_panel_err_t lvgl_touch_init(void)
{
    const touch_panel_err_t err = touch_panel_init(TOUCH_PANEL_ROTATION_90);
    if (unlikely(err != TOUCH_PANEL_OK)) {
        return err;
    }

    lv_indev_drv_init(&lvgl_ctx.indev_drv);
    lvgl_ctx.indev_drv.type = LV_INDEV_TYPE_POINTER; // Touchpad
	lvgl_ctx.indev_drv.read_cb = lvgl_on_input_read;

    /* Register input device driver */
	lv_indev_drv_register(&lvgl_ctx.indev_drv);

    return TOUCH_PANEL_OK;
}

static esp_err_t lvgl_tick_timer_init(void)
{
    const esp_timer_create_args_t timer_args = {
        .callback = lvgl_tick_timer_callback,
        .name = LVGL_TICK_TIMER_NAME
    };

    esp_timer_handle_t timer;

    esp_err_t err = esp_timer_create(&timer_args, &timer);
    if (unlikely(err != ESP_OK)) {
        return err;
    }
    err = esp_timer_start_periodic(timer, LVGL_TICK_TIMER_PERIOD_MS * 1000);
    if (unlikely(err != ESP_OK)) {
        esp_timer_delete(timer);
        return err;
    }
    return ESP_OK;
}

static esp_err_t lvgl_eink_sleep_timer_init(void)
{
    lvgl_ctx.eink_sleep_timer = xTimerCreate(LVGL_SLEEP_TIMER_NAME, pdMS_TO_TICKS(LVGL_SLEEP_TIMER_PERIOD_MS), pdFALSE, NULL, lvgl_eink_sleep_timer_callback);
    if (unlikely(lvgl_ctx.eink_sleep_timer == NULL)) {
        return ESP_FAIL;
    }
    if (unlikely(xTimerStart(lvgl_ctx.eink_sleep_timer, 0) != pdPASS)) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

static eink_err_t lvgl_eink_wakeup(void)
{
    /* Restart sleep timer */
    xTimerReset(lvgl_ctx.eink_sleep_timer, 0);

    /* If eink asleep - wake it up */
    if (!lvgl_ctx.is_eink_awake) {
        ESP_LOGI("", "Waking up eink!");
        const esp_err_t err = eink_wakeup();
        if (unlikely(err != ESP_OK)) {
            ESP_LOGE("", "Failed to wakeup eink: %d", err);
            return err;
        }
        lvgl_ctx.is_eink_awake = true;
    }

    return ESP_OK;
}

static void lvgl_task(void *arg)
{
    while (1) {
        if (lvgl_task_acquire()) {
            lv_task_handler();
            lvgl_task_release();
            vTaskDelay(pdMS_TO_TICKS(LVGL_TASK_HANDLER_PERIOD_MS));
        }
    }
}

static void lvgl_tick_timer_callback(void *arg)
{
    lv_tick_inc(LVGL_TICK_TIMER_PERIOD_MS);
}

static void lvgl_eink_sleep_timer_callback(TimerHandle_t timer)
{
    ESP_LOGI("", "Putting eink to sleep!");

    const eink_err_t err = eink_sleep();
    if (unlikely(err != EINK_OK)) {
        ESP_LOGE("", "Failed to put eink to sleep: %d", err);
        return;
    }

    lvgl_ctx.is_eink_awake = false;
}
