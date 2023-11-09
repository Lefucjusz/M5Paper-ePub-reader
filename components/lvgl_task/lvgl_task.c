#include "lvgl_task.h"
#include "utils.h"
#include "touch.h"
#include <lvgl.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>

struct lvgl_ctx_t
{
    lv_disp_draw_buf_t disp_buf;
	lv_color_t draw_buf[LVGL_DRAW_BUFFER_SIZE];
	lv_disp_drv_t disp_drv;
	lv_indev_drv_t indev_drv;
};

static struct lvgl_ctx_t lvgl_ctx;

/* Private functions declarations */
static void lvgl_transform_pixel_map(lv_color_t *px_map, size_t size);
static void lvgl_on_display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *px_map);
static void lvgl_coords_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area);
static void lvgl_on_input_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

static eink_err_t lvgl_display_init(void);
static touch_err_t lvgl_touch_init(void);
static esp_err_t lvgl_timer_init(void);

static void lvgl_task(void *arg);
static void lvgl_tick_callback(void *arg);

/* Public functions */
void lvgl_init(void)
{
    /* Clear context */
    memset(&lvgl_ctx, 0, sizeof(lvgl_ctx));

    /* Initialize LVGL */
    lv_init();

    /* Initialize hardware */
    lvgl_display_init(); // TODO error handling
    lvgl_touch_init();
    lvgl_timer_init();
}

void lvgl_task_start(void)
{
    xTaskCreatePinnedToCore(lvgl_task, LVGL_TASK_NAME, LVGL_TASK_STACK_SIZE / sizeof(StackType_t), NULL, 0, NULL, LVGL_TASK_CORE_AFFINITY);
}

/* Private functions */
static void lvgl_transform_pixel_map(lv_color_t *px_map, size_t size)
{
    /* Hack - reduce number of passes by processing 4 pixels at once using uint32_t */
    const size_t iterations = size / sizeof(uint32_t);
    const uint32_t *src = (const uint32_t *)px_map;
    uint16_t *dst = (uint16_t *)px_map;
    // uint8_t pixels[4];

    /* Transform */
    for (size_t i = 0; i < iterations; ++i) {
        lv_color_t color;

        color.full = (src[i] >> 24) & 0xFF;
        const uint8_t pixel0 = lv_color_brightness(color) / 16;

        color.full = (src[i] >> 16) & 0xFF;
        const uint8_t pixel1 = lv_color_brightness(color) / 16;

        color.full = (src[i] >> 8) & 0xFF;
        const uint8_t pixel2 = lv_color_brightness(color) / 16;

        color.full = (src[i] >> 0) & 0xFF;
        const uint8_t pixel3 = lv_color_brightness(color) / 16;

        dst[i] = MAKE_WORD(MAKE_BYTE(pixel1, pixel0), MAKE_BYTE(pixel3, pixel2));
    }
} 

static void lvgl_on_display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *px_map)
{
    // static uint32_t last_refresh;

    const size_t dx = (area->x2 - area->x1) + 1;
	const size_t dy = (area->y2 - area->y1) + 1;

    lvgl_transform_pixel_map(px_map, dx * dy);
    eink_write(area->x1, area->y1, dx, dy, (uint8_t *)px_map);

    if (lv_disp_flush_is_last(disp_drv)) {
        eink_refresh_full(EINK_UPDATE_MODE_GL16);
        // uint32_t current_refresh = tick_cnt;
        // ESP_LOGE("PROFILING", "Time between refreshes: %lums", current_refresh - last_refresh);
        // last_refresh = current_refresh;
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
    struct touch_coords_t coords;
    touch_get_coords(&coords);

    data->point.x = coords.x;
    data->point.y = coords.y;
    data->state = coords.state;
}

static eink_err_t lvgl_display_init(void)
{
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
    lv_disp_drv_register(&lvgl_ctx.disp_drv);

    return EINK_OK;
}

static touch_err_t lvgl_touch_init(void)
{
    const touch_err_t err = touch_init(TOUCH_ROTATION_90);
    if (unlikely(err != TOUCH_OK)) {
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(1000)); // TODO sometimes init fails even with this delay, investigate

    lv_indev_drv_init(&lvgl_ctx.indev_drv);
    lvgl_ctx.indev_drv.type = LV_INDEV_TYPE_POINTER; // Touchpad
	lvgl_ctx.indev_drv.read_cb = lvgl_on_input_read;

    /* Register input device driver */
	lv_indev_drv_register(&lvgl_ctx.indev_drv);

    return TOUCH_OK;
}

static esp_err_t lvgl_timer_init(void)
{
    const esp_timer_create_args_t timer_args = {
        .callback = lvgl_tick_callback,
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

static void lvgl_task(void *arg)
{
    while (1) {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(LVGL_TASK_HANDLER_PERIOD_MS));
    }
}

static void lvgl_tick_callback(void *arg)
{
    lv_tick_inc(LVGL_TICK_TIMER_PERIOD_MS);
}
