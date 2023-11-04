#include <stdio.h>
#include "eink.h"
#include "utils.h"
#include "touch.h"
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

#define LVGL_DRAW_BUFFER_SIZE_PIXELS (EINK_DISPLAY_WIDTH * 200) // 200 lines
#define LVGL_DRAW_BUFFER_SIZE_BYTES (LVGL_DRAW_BUFFER_SIZE_PIXELS / EINK_PIXELS_PER_BYTE)

#define LV_TICK_PERIOD_MS 1

static uint32_t tick_cnt = 0;

struct lvgl_ctx_t
{
    lv_disp_draw_buf_t disp_buf;
	lv_color_t draw_buf[LVGL_DRAW_BUFFER_SIZE_BYTES];
	lv_disp_drv_t disp_drv;
	lv_indev_drv_t indev_drv;
};

static struct lvgl_ctx_t lvgl_ctx;

static void gui_task(void *arg);
static void lv_tick_task(void *arg);

static void lvgl_on_display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *px_map)
{
    const size_t dx = (area->x2 - area->x1) + 1;
	const size_t dy = (area->y2 - area->y1) + 1;

    uint32_t write_start = tick_cnt;
    eink_write(area->x1, area->y1, dx, dy, (uint8_t *)px_map);
    uint32_t write_end = tick_cnt;
    ESP_LOGE("PROFILING", "eink_write time: %lums", write_end - write_start);
    // eink_refresh(area->x1, area->y1, dx, dy, EINK_UPDATE_MODE_DU4);

    if (lv_disp_flush_is_last(disp_drv)) {
        uint32_t refresh_start = tick_cnt;
        eink_refresh_full(EINK_UPDATE_MODE_DU4);
        uint32_t refresh_end = tick_cnt;
        ESP_LOGE("PROFILING", "eink_refresh_full time: %lums", refresh_end - refresh_start);
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

static void lvgl_on_input_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    struct touch_coords_t coords;
    touch_get_coords(&coords);

    data->point.x = coords.x;
    data->point.y = coords.y;
    data->state = coords.state;
}

static void lvgl_touch_init(void)
{
    touch_init(TOUCH_ROTATION_90);
    vTaskDelay(pdMS_TO_TICKS(1000)); // TODO sometimes init fails even with this delay, investigate

    lv_indev_drv_init(&lvgl_ctx.indev_drv);
    lvgl_ctx.indev_drv.type = LV_INDEV_TYPE_POINTER; // Touchpad
	lvgl_ctx.indev_drv.read_cb = lvgl_on_input_read;

    /* Register input device driver */
	lv_indev_drv_register(&lvgl_ctx.indev_drv);
}

static void lvgl_init(void)
{
    /* Initialize display and LVGL */
    eink_err_t e = eink_init(EINK_ROTATION_90, EINK_COLOR_NORMAL);
    if (e) {
        ESP_LOGE("", "%d", e);
        while (1);
    }
    lv_init();

    // lvgl_ctx.draw_buf = heap_caps_malloc(LVGL_DRAW_BUFFER_SIZE_BYTES, MALLOC_CAP_DMA);
    // if (lvgl_ctx.draw_buf == NULL) {
    //     ESP_LOGE(__FUNCTION__, "malloc failed!");
    //     while (1);
    // }

    /* Initialize display buffer */
    lv_disp_draw_buf_init(&lvgl_ctx.disp_buf, &lvgl_ctx.draw_buf, NULL, LVGL_DRAW_BUFFER_SIZE_PIXELS);

    /* Create display driver */
    lv_disp_drv_init(&lvgl_ctx.disp_drv);
    lvgl_ctx.disp_drv.draw_buf = &lvgl_ctx.disp_buf;
    lvgl_ctx.disp_drv.flush_cb = lvgl_on_display_flush;
    lvgl_ctx.disp_drv.rounder_cb = lvgl_coords_rounder;
    lvgl_ctx.disp_drv.set_px_cb = lvgl_on_set_px;
	lvgl_ctx.disp_drv.hor_res = EINK_DISPLAY_HEIGHT;
	lvgl_ctx.disp_drv.ver_res = EINK_DISPLAY_WIDTH;

    /* Register display driver */
    lv_disp_drv_register(&lvgl_ctx.disp_drv);

    lvgl_touch_init();
}

void app_main(void)
{
    xTaskCreatePinnedToCore(gui_task, "gui", 4096 * 2, NULL, 0, NULL, 1);
}

static void button_cb(lv_event_t *event)
{
    ESP_LOGI("", "CLICKED!");
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

    lv_obj_t * list1 = lv_list_create(lv_scr_act());
    lv_obj_set_size(list1, 540, 960);
    lv_obj_align(list1, LV_ALIGN_TOP_MID, 0, 0);

    /*Add buttons to the list*/
    lv_obj_t * list_btn;

    list_btn = lv_list_add_btn(list1, LV_SYMBOL_FILE, "New item");
    lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    list_btn = lv_list_add_btn(list1, LV_SYMBOL_DIRECTORY, "Test item");
    lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    list_btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Delete item");
    lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    list_btn = lv_list_add_btn(list1, LV_SYMBOL_EDIT, "Some very long text to test");
    lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    list_btn = lv_list_add_btn(list1, LV_SYMBOL_SAVE, "Save item");
    lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    list_btn = lv_list_add_btn(list1, LV_SYMBOL_BELL, "Notify item");
    lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
    for (size_t i = 0; i < 36; ++i) {
        char buf[32];
        snprintf(buf, 32, "Element with callback %u", i+1);
        list_btn = lv_list_add_btn(list1, LV_SYMBOL_BATTERY_FULL, buf);
        lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
        lv_obj_add_event_cb(list_btn, button_cb, LV_EVENT_CLICKED, NULL);
    }
    
   

    while (1) {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

}

static void lv_tick_task(void *arg)
{
    lv_tick_inc(LV_TICK_PERIOD_MS);
    tick_cnt++;
}
