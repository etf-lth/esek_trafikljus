
#include "fan.h"

#include "driver/ledc.h"

void Fan::init()
{
    ledc_timer_config_t fan_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_12_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 100,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&fan_timer));

    ledc_channel_config_t fan_channel = {
        .gpio_num = 16,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0, // Set duty to 0%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&fan_channel));
}

void Fan::setSpeed(unsigned int val)
{
    unsigned int duty = val / 100 * (1 << 12);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}