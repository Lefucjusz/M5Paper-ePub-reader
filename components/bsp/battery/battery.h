#pragma once

#include <esp_err.h>
#include <stdint.h>

#define BATTERY_ADC_SAMPLES_TO_AVERAGE 16

void battery_init(void);

uint16_t battery_get_raw_adc_value(void);
uint16_t battery_get_voltage(void);
uint16_t battery_get_voltage_filtered(void);
uint8_t battery_get_percent(void);
