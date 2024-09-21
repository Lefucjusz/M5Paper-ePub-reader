#include "sleep.h"
#include <touch_panel.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

esp_err_t sleep_init(void)
{
    const gpio_config_t config = {
        .pin_bit_mask = (1ULL << TOUCH_PANEL_IRQ_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = false,
        .pull_up_en = false,
        .intr_type = GPIO_INTR_DISABLE
    };

    esp_err_t err = gpio_config(&config);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_wakeup_enable(TOUCH_PANEL_IRQ_PIN, GPIO_INTR_LOW_LEVEL);
    if (err != ESP_OK) {
        return err;
    }

    return esp_sleep_enable_gpio_wakeup();
}

esp_err_t sleep_enter(void)
{
    return esp_light_sleep_start();
}
