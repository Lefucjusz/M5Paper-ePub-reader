#include "sleep.h"
#include <touch_panel.h>
#include <esp_sleep.h>

esp_err_t sleep_init(void)
{
    const esp_err_t err = gpio_wakeup_enable(TOUCH_PANEL_IRQ_PIN, GPIO_INTR_LOW_LEVEL);
    if (err != ESP_OK) {
        return err;
    }

    return esp_sleep_enable_gpio_wakeup();
}

esp_err_t sleep_enter(void)
{
    return esp_light_sleep_start();
}
