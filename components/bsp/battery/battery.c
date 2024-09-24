#include "battery.h"
#include <math.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <soc/adc_channel.h>

#define MILLIVOLTS_TO_VOLTS(x) ((x) / 1000.0f)

#define BATTERY_ADC_UNIT ADC_UNIT_1
#define BATTERY_ADC_CHANNEL ADC1_GPIO35_CHANNEL
#define BATTERY_ADC_BIT_WIDTH ADC_WIDTH_BIT_12
#define BATTERY_ADC_DEFAULT_VREF 1100
#define BATTERY_ADC_ATTENUATION ADC_ATTEN_DB_12

/* Voltage from the battery is fed to ADC pin via a voltage divider. According to 
M5Paper schematic, the values of the resistors used in the divider are 3k and 11k, 
what gives attenuation factor of (11k + 3k) / 11k = ~1.27x. The real values seem 
to be different though - both are reportedly equal to 10k 
(see https://community.m5stack.com/topic/2581/m5paper-epd-power-consumption/30).
Hence, the attenuation factor is equal to (10k + 10k) / 10k = 2x. */
#define BATTERY_VOLTAGE_DIVIDER_ATTENUATION 2
#define BATTERY_VOLTAGE_MAX 4200
#define BATTERY_VOLTAGE_MIN 3500

static esp_adc_cal_characteristics_t adc_cal_data;

void battery_init(void)
{
    adc1_config_width(BATTERY_ADC_BIT_WIDTH);
    adc1_config_channel_atten(BATTERY_ADC_CHANNEL, BATTERY_ADC_ATTENUATION);
    esp_adc_cal_characterize(BATTERY_ADC_UNIT, BATTERY_ADC_ATTENUATION, BATTERY_ADC_BIT_WIDTH, BATTERY_ADC_DEFAULT_VREF, &adc_cal_data);
}

uint16_t battery_get_raw_adc_value(void)
{
    return adc1_get_raw(BATTERY_ADC_CHANNEL);
}

uint16_t battery_get_voltage(void)
{
    const uint16_t adc_value = battery_get_raw_adc_value();
    const uint16_t voltage = esp_adc_cal_raw_to_voltage(adc_value, &adc_cal_data) * BATTERY_VOLTAGE_DIVIDER_ATTENUATION;
    return voltage;
}

uint16_t battery_get_voltage_filtered(void)
{
    uint32_t voltage_avg = 0;

    for (size_t i = 0; i < BATTERY_ADC_SAMPLES_TO_AVERAGE; ++i) {
        voltage_avg += battery_get_voltage();
    }
    voltage_avg /= BATTERY_ADC_SAMPLES_TO_AVERAGE;
    
    return voltage_avg;
}

/* See here for detailed explanation: 
 * https://github.com/G6EJD/ESP32-e-Paper-Weather-Display/issues/146
 * TL;DR: this is 4th order polynomial used to map voltage to percentage, 
 * based on li-ion discharge curve. */
uint8_t battery_get_percent(void)
{
    const uint16_t voltage_mv = battery_get_voltage_filtered();
    if (voltage_mv >= BATTERY_VOLTAGE_MAX) {
        return 100;
    } 
    if (voltage_mv <= BATTERY_VOLTAGE_MIN) {
        return 0;
    }

    const float voltage_v = MILLIVOLTS_TO_VOLTS(voltage_mv);
    const float a = 2836.9625f;
    const float b = 43987.4889f;
    const float c = 255233.8134f;
    const float d = 656689.7123f;
    const float e = 632041.7303f;
    return roundf((a * powf(voltage_v, 4)) - (b * powf(voltage_v, 3)) + (c * powf(voltage_v, 2)) - (d * voltage_v) + e);
}
