#include <stdio.h>
#include "eink.h"
#include "utils.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lvgl/lvgl.h>

/* Alignment:
* - h doesn't need to be aligned in any rotation
* - w always has to be aligned
* - x always has to be aligned
* - y doesn't need to be aligned in any rotation
*/

#define LVGL_DRAW_BUFFER_SIZE_PIXELS ((EINK_DISPLAY_WIDTH * EINK_DISPLAY_HEIGHT) / 10)
#define LVGL_DRAW_BUFFER_SIZE_BYTES (LVGL_DRAW_BUFFER_SIZE_PIXELS / EINK_PIXELS_PER_BYTE)

#define LV_TICK_PERIOD_MS 1

struct lvgl_ctx_t
{
    lv_disp_draw_buf_t disp_buf;
	lv_color_t *draw_buf;
	lv_disp_drv_t disp_drv;
	// lv_indev_drv_t indev_drv;
};

static struct lvgl_ctx_t lvgl_ctx;

static void gui_task(void *arg);
static void lv_tick_task(void *arg);

static void lvgl_on_display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *px_map)
{
    const size_t dx = (area->x2 - area->x1) + 1;
	const size_t dy = (area->y2 - area->y1) + 1;

    eink_write(area->x1, area->y1, dx, dy, (uint8_t *)px_map);
    // eink_refresh(area->x1, area->y1, dx, dy, EINK_UPDATE_MODE_DU);

    if (lv_disp_flush_is_last(disp_drv)) {
        eink_refresh_full(EINK_UPDATE_MODE_GC16);
    }

    lv_disp_flush_ready(&lvgl_ctx.disp_drv);
}

static void lvgl_coords_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    /* Align x1 and width to be multiple of 4 */
    area->x1 &= ~(EINK_WIDTH_PIXELS_ALIGNMENT - 1);
    area->x2 |= (EINK_WIDTH_PIXELS_ALIGNMENT - 1);
}

static void lvgl_on_set_px(lv_disp_drv_t *disp_drv, uint8_t *buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{
    /* For RGB232 value of 252 describes white */
    const uint8_t px_brightness = map(lv_color_brightness(color), 0, 252, EINK_PIXEL_BLACK, EINK_PIXEL_WHITE);
    const size_t byte_index = (buf_w * y + x) / EINK_PIXELS_PER_BYTE;
    
    if (x & 1) {
        buf[byte_index] &= ~EINK_PIXEL_WHITE; // Handle odd pixel
        buf[byte_index] |= px_brightness;
    }
    else {
        buf[byte_index] &= ~(EINK_PIXEL_WHITE << 4); // Handle even pixel
        buf[byte_index] |= (px_brightness << 4);
    }
}

// static void lvgl_on_flush_done(lv_disp_drv_t *disp_drv, uint32_t time, uint32_t px)
// {
//     ESP_LOGI("MAIN", "time: %lu, px: %lu", time, px);
//     eink_refresh_full(EINK_UPDATE_MODE_GC16);
// }

static void lvgl_init(void)
{
    /* Initialize display and LVGL */
    eink_err_t e = eink_init(EINK_ROTATION_90, EINK_COLOR_NORMAL);
    if (e) {
        ESP_LOGE("", "%d", e);
        while (1);
    }
    lv_init();

    lvgl_ctx.draw_buf = heap_caps_malloc(LVGL_DRAW_BUFFER_SIZE_BYTES, MALLOC_CAP_DMA);
    if (lvgl_ctx.draw_buf == NULL) {
        ESP_LOGE(__FUNCTION__, "malloc failed!");
        while (1);
    }

    /* Initialize display buffer */
    lv_disp_draw_buf_init(&lvgl_ctx.disp_buf, lvgl_ctx.draw_buf, NULL, LVGL_DRAW_BUFFER_SIZE_PIXELS);

    /* Create display driver */
    lv_disp_drv_init(&lvgl_ctx.disp_drv);
    lvgl_ctx.disp_drv.draw_buf = &lvgl_ctx.disp_buf;
    lvgl_ctx.disp_drv.flush_cb = lvgl_on_display_flush;
    lvgl_ctx.disp_drv.rounder_cb = lvgl_coords_rounder;
    lvgl_ctx.disp_drv.set_px_cb = lvgl_on_set_px;
    // lvgl_ctx.disp_drv.monitor_cb = lvgl_on_flush_done; // TODO hack
	lvgl_ctx.disp_drv.hor_res = EINK_DISPLAY_HEIGHT;
	lvgl_ctx.disp_drv.ver_res = EINK_DISPLAY_WIDTH;

    /* Register display driver */
    lv_disp_drv_register(&lvgl_ctx.disp_drv);
}

void app_main(void)
{
   xTaskCreatePinnedToCore(gui_task, "gui", 4096 * 2, NULL, 0, NULL, 1);
}

static void gui_task(void *arg)
{
    lvgl_init();

    const esp_timer_create_args_t timer_args = {
        .callback = lv_tick_task,
        .name = "periodic_gui"
    };

    esp_timer_handle_t timer;

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, LV_TICK_PERIOD_MS * 1000));

    lv_obj_t *text = lv_label_create(lv_scr_act());
    lv_obj_set_size(text, 200, 60);
    lv_label_set_long_mode(text, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_font(text, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_label_set_text(text, "Hello from LVGL!");
    lv_obj_center(text);

    // lv_obj_t *label2 = lv_label_create(lv_scr_act());
    // lv_obj_set_size(label2, 100, 60);
    // lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);
    // lv_obj_set_style_text_font(label2, &lv_font_montserrat_14, LV_PART_MAIN);
    // lv_label_set_text(label2, "Hello from LVGL, but smaller...");
    // lv_obj_align(label2, LV_ALIGN_CENTER, 30, 70);

    lv_obj_t *button = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button, 200, 200);
    lv_obj_align_to(button, text, LV_ALIGN_OUT_BOTTOM_MID, 0, 50);
    
    lv_obj_t *label = lv_label_create(button);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_label_set_text(label, "Button!");
    lv_obj_center(label);

   

    while (1) {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

}

static void lv_tick_task(void *arg)
{
    lv_tick_inc(LV_TICK_PERIOD_MS);
}
