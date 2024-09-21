#include "eink_worker.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#define TAG __FILENAME__

typedef enum
{
    EINK_TASK_WRITE,
    EINK_TASK_REFRESH
} eink_op_type_t;

typedef struct
{
    eink_op_type_t type;

    lv_color_t *px_map;
    size_t x;
    size_t y;
    size_t w;
    size_t h;
} eink_op_t;

typedef struct
{
    void (*on_ready)(void);
    QueueHandle_t operation_queue;
    uint8_t fast_refresh_count;
    bool is_busy;
} eink_worker_ctx;

static eink_worker_ctx ctx;

static void eink_worker_refresh_screen(void);
static void eink_worker_transform_pixel_map(lv_color_t *px_map, size_t size);
static void eink_worker(void *arg);

eink_err_t eink_worker_start(void (*on_ready)(void))
{
    ctx.on_ready = on_ready;

    ctx.operation_queue = xQueueCreate(EINK_WORKER_OPERATION_QUEUE_LENGTH, sizeof(eink_op_t));
    if (ctx.operation_queue == NULL) {
        return EINK_NO_MEMORY;
    }

    xTaskCreatePinnedToCore(eink_worker, EINK_WORKER_NAME, EINK_WORKER_STACK_SIZE / sizeof(StackType_t), NULL, 0, NULL, EINK_WORKER_CORE_AFFINITY);
    return EINK_OK;
}

void eink_worker_write(size_t x, size_t y, size_t w, size_t h, lv_color_t *px_map)
{
    eink_op_t operation = {
        .type = EINK_TASK_WRITE,
        .x = x,
        .y = y,
        .w = w,
        .h = h,
        .px_map = px_map
    };
   
    if (xQueueSend(ctx.operation_queue, &operation, portMAX_DELAY) == pdFALSE) { // TODO timeout handling
        ESP_LOGE(TAG, "Failed to push write item to queue!");
    }
}

void eink_worker_refresh(void)
{
    eink_op_t operation = {
        .type = EINK_TASK_REFRESH
    };

    if (xQueueSend(ctx.operation_queue, &operation, portMAX_DELAY) == pdFALSE) { // TODO timeout handling
        ESP_LOGE(TAG, "Failed to push refresh item to queue!");
    }
}

bool eink_worker_idle(void)
{
    return !ctx.is_busy;
}

/* Private functions */
static void eink_worker_refresh_screen(void)
{
    if (ctx.fast_refresh_count >= EINK_WORKER_FAST_PER_DEEP_REFRESHES) {
        ESP_LOGI(TAG, "Refreshing with GC16");
        eink_refresh_full(EINK_UPDATE_MODE_GC16);
        ctx.fast_refresh_count = 0;
    }
    else {
        ESP_LOGI(TAG, "Refreshing with A2");
        eink_refresh_full(EINK_UPDATE_MODE_A2); // TODO maybe switch to DU
        ctx.fast_refresh_count++;
    }
}

static void eink_worker_transform_pixel_map(lv_color_t *px_map, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        const size_t byte_index = i / EINK_PIXELS_PER_BYTE;
        const uint8_t px_index = i % EINK_PIXELS_PER_BYTE;
        const uint8_t px_brightness = (px_map[i].full > 0) ? EINK_PIXEL_WHITE : EINK_PIXEL_BLACK; // Only monochrome supported
        
        if (px_index) {
            px_map[byte_index].full &= ~EINK_PIXEL_WHITE;
            px_map[byte_index].full |= px_brightness;
        }
        else {
            px_map[byte_index].full &= ~(EINK_PIXEL_WHITE << 4);
            px_map[byte_index].full |= (px_brightness << 4);
        }
    }
}

static void eink_worker(void *arg)
{
    ESP_LOGI(TAG, "eink_worker started!");

    /* Set initial state */
    ctx.fast_refresh_count = 0;
    ctx.is_busy = false;

    /* Main loop */
    while (1) {
        eink_op_t operation;
        if (xQueueReceive(ctx.operation_queue, &operation, portMAX_DELAY) == pdFALSE) {
            ESP_LOGE(TAG, "Failed to receive operation item from queue!");
            continue;
        }

        ctx.is_busy = true;

        switch (operation.type) {
            case EINK_TASK_WRITE:
                eink_worker_transform_pixel_map(operation.px_map, operation.w * operation.h);
                eink_write(operation.x, operation.y, operation.w, operation.h, (const uint8_t *)operation.px_map);
                break;
            
            case EINK_TASK_REFRESH:
                eink_worker_refresh_screen();
                break;

            default:
                ESP_LOGE(TAG, "Received invalid operation type: %d", operation.type);
                break;
        }

        if ((uxQueueMessagesWaiting(ctx.operation_queue) == 0) && (ctx.on_ready != NULL)) {
            ctx.on_ready();
            ctx.is_busy = false;
        }
    }
}
