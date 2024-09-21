#include "lvgl_task.h"
#include <utils.h>
#include <sleep.h>
#include <touch_panel.h>
#include <lvgl.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_check.h>
#include <freertos/FreeRTOS.h>

#define TAG LVGL_TASK_NAME

typedef struct
{
    lv_disp_draw_buf_t disp_buf;
	lv_color_t draw_buf[LVGL_DRAW_BUFFER_SIZE];
	lv_disp_drv_t disp_drv;
	lv_indev_drv_t indev_drv;
    eink_update_mode_t refresh_mode;
    uint8_t fast_refresh_count;
    esp_timer_handle_t tick_timer;
} lvgl_ctx_t;

static lvgl_ctx_t /* EXT_RAM_BSS_ATTR */ ctx;

/* Private functions declarations */
static void lvgl_transform_pixel_map(lv_color_t *px_map, size_t size);
static void lvgl_refresh_screen(void);
static void lvgl_on_display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *px_map);
static void lvgl_coords_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area);
static void lvgl_on_input_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

static eink_err_t lvgl_display_init(void);
static touch_panel_err_t lvgl_touch_init(void);
static esp_err_t lvgl_tick_timer_init(void);

static void lvgl_enter_sleep_mode(void);
static void lvgl_leave_sleep_mode(void);

static void lvgl_task(void *arg);
static void lvgl_tick_timer_callback(void *arg);

/* Public functions */
void lvgl_task_init(void)
{
    /* Initialize LVGL */
    lv_init();

    /* Initialize hardware */
    lvgl_display_init(); // TODO error handling
    lvgl_touch_init();
    lvgl_tick_timer_init();
}

void lvgl_task_start(void)
{
    xTaskCreatePinnedToCore(lvgl_task, LVGL_TASK_NAME, LVGL_TASK_STACK_SIZE / sizeof(StackType_t), NULL, 0, NULL, LVGL_TASK_CORE_AFFINITY);
}

/* Private functions */
static void lvgl_reset_refresh_mode(void)
{
    /* Default refresh mode */
    ctx.refresh_mode = EINK_UPDATE_MODE_A2;
}

static void lvgl_update_refresh_mode(uint8_t px_brightness)
{
    /* The most capable refresh mode already required, return */
    if (ctx.refresh_mode == EINK_UPDATE_MODE_GC16) {
        return;
    }

    /* If not handled by A2 */
    if ((px_brightness != EINK_PIXEL_BLACK) && (px_brightness != EINK_PIXEL_WHITE)) {
        if ((px_brightness == EINK_PIXEL_DARK_GRAY_DU4) || (px_brightness == EINK_PIXEL_LIGHT_GRAY_DU4)) {
            ctx.refresh_mode = EINK_UPDATE_MODE_DU4; // Can be handled by DU4
        }
        else {
            ESP_LOGW("", "%d", px_brightness);
            ctx.refresh_mode = EINK_UPDATE_MODE_GC16; // Can be handled only by GC16
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
    if ((ctx.refresh_mode == EINK_UPDATE_MODE_GC16) || (ctx.fast_refresh_count >= LVGL_FAST_PER_DEEP_REFRESHES)) {
        ESP_LOGI(TAG, "Refreshing with GC16");
        eink_refresh_full(EINK_UPDATE_MODE_GC16);
        ctx.fast_refresh_count = 0;
    }
    else {
        ESP_LOGI(TAG, "Refreshing with %s", ctx.refresh_mode == EINK_UPDATE_MODE_DU4 ? "DU4" : "A2");
        eink_refresh_full(ctx.refresh_mode);
        ++ctx.fast_refresh_count;
    }
}

static void lvgl_on_display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *px_map)
{
    static uint32_t last_refresh;

    const size_t width = (area->x2 - area->x1) + 1;
	const size_t height = (area->y2 - area->y1) + 1;
    const size_t area_size = width * height;

    lvgl_transform_pixel_map(px_map, area_size);
    eink_write(area->x1, area->y1, width, height, (uint8_t *)px_map);

    if (lv_disp_flush_is_last(disp_drv)) {
        lvgl_refresh_screen();
        lvgl_reset_refresh_mode();
        const uint32_t current_refresh = lv_tick_get();
        ESP_LOGI(TAG, "Time between refreshes: %lums", current_refresh - last_refresh);
        last_refresh = current_refresh;
    }

    lv_disp_flush_ready(&ctx.disp_drv); // TODO the rendering concept is not very good, try to parallelize drawing new frame and rendering previous
}

static void lvgl_coords_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    /* Align x1 and width to be multiple of 4 */
    area->x1 &= ~(EINK_WIDTH_PIXELS_ALIGNMENT - 1);
    area->x2 |= (EINK_WIDTH_PIXELS_ALIGNMENT - 1);
}

static void lvgl_on_input_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    touch_panel_coords_t coords;
    touch_get_coords(&coords);

    data->point.x = coords.x;
    data->point.y = coords.y;
    data->state = coords.state;
}

static eink_err_t lvgl_display_init(void)
{
    /* Set initial state */
    ctx.fast_refresh_count = 0;
    lvgl_reset_refresh_mode();

    /* Initialize eink hardware */
    const eink_err_t err = eink_init(EINK_ROTATION_90, EINK_COLOR_NORMAL);
    if (err) {
       return err;
    }

    /* Initialize display buffer */
    lv_disp_draw_buf_init(&ctx.disp_buf, &ctx.draw_buf, NULL, LVGL_DRAW_BUFFER_SIZE);

    /* Create display driver */
    lv_disp_drv_init(&ctx.disp_drv);
    ctx.disp_drv.draw_buf = &ctx.disp_buf;
    ctx.disp_drv.flush_cb = lvgl_on_display_flush;
    ctx.disp_drv.rounder_cb = lvgl_coords_rounder;
	ctx.disp_drv.hor_res = EINK_DISPLAY_HEIGHT;
	ctx.disp_drv.ver_res = EINK_DISPLAY_WIDTH;

    /* Register display driver */
    lv_disp_t *disp = lv_disp_drv_register(&ctx.disp_drv);

    /* Initialize default theme */
    if (lv_theme_mono_is_inited() == false) {
        disp->theme = lv_theme_mono_init(disp, false, LV_FONT_DEFAULT);
    }

    return EINK_OK;
}

static touch_panel_err_t lvgl_touch_init(void)
{
    const touch_panel_err_t err = touch_panel_init(TOUCH_PANEL_ROTATION_90);
    if (err) {
        return err;
    }

    lv_indev_drv_init(&ctx.indev_drv);
    ctx.indev_drv.type = LV_INDEV_TYPE_POINTER; // Touchpad
	ctx.indev_drv.read_cb = lvgl_on_input_read;

    /* Register input device driver */
	lv_indev_drv_register(&ctx.indev_drv);

    return TOUCH_PANEL_OK;
}

static esp_err_t lvgl_tick_timer_init(void)
{
    const esp_timer_create_args_t timer_args = {
        .callback = lvgl_tick_timer_callback,
        .name = LVGL_TICK_TIMER_NAME
    };

    esp_err_t err = esp_timer_create(&timer_args, &ctx.tick_timer);
    if (err) {
        return err;
    }
    err = esp_timer_start_periodic(ctx.tick_timer, LVGL_TICK_TIMER_PERIOD_MS * 1000);
    if (err) {
        esp_timer_delete(ctx.tick_timer);
        return err;
    }
    return ESP_OK;
}

static void lvgl_enter_sleep_mode(void)
{
    const eink_err_t eink_err = eink_sleep();
    if (eink_err != EINK_OK) {
        ESP_LOGE(TAG, "Failed to put eink to sleep, error %d", eink_err);
        return;
    }

    esp_err_t esp_err = esp_timer_stop(ctx.tick_timer);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop tick timer, error %d", esp_err);
    }

    esp_err = sleep_enter();
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to put ESP to light sleep, error %d", esp_err);
    }
}

static void lvgl_leave_sleep_mode(void)
{
    const esp_err_t timer_err = esp_timer_start_periodic(ctx.tick_timer, LVGL_TICK_TIMER_PERIOD_MS * 1000);
    if (timer_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start tick timer, error %d", timer_err);
        return;
    }

    const eink_err_t eink_err = eink_wakeup();
    if (eink_err != EINK_OK) {
        ESP_LOGE(TAG, "Failed to wake up eink, error %d", eink_err);
        return;
    }

    lv_tick_inc(LV_DISP_DEF_REFR_PERIOD);
    lv_task_handler();
}

static void lvgl_task(void *arg)
{
    sleep_init();

    while (1) {
        if (lv_disp_get_inactive_time(NULL) < LVGL_SLEEP_INACTIVITY_PERIOD_MS) {
            lv_task_handler();
        }
        else {
            lvgl_enter_sleep_mode();

            // Here CPU is sleeping

            lvgl_leave_sleep_mode();
        }
        vTaskDelay(pdMS_TO_TICKS(LVGL_TASK_HANDLER_PERIOD_MS));
    }
}

static void lvgl_tick_timer_callback(void *arg)
{
    lv_tick_inc(LVGL_TICK_TIMER_PERIOD_MS);
}
