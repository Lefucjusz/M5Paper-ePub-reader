#include "sleep.h"
#include <touch_panel.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

esp_err_t sleep_gpio_init(void)
{
    const esp_err_t err = gpio_wakeup_enable(TOUCH_PANEL_IRQ_PIN, GPIO_INTR_LOW_LEVEL);
    if (err != ESP_OK) {
        return err;
    }

    return esp_sleep_enable_gpio_wakeup();
}

esp_err_t sleep_timer_init(uint64_t time_us)
{
    return esp_sleep_enable_timer_wakeup(time_us);
}

esp_err_t sleep_enter(void)
{
    return esp_light_sleep_start();
}
