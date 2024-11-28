
#include "fan.h"

#include "driver/ledc.h"

void Fan::init(int gpioNum, uint32_t freqHz, uint32_t duty)
{
    ledc_timer_config_t fan_timer = {};
    fan_timer.speed_mode = LEDC_LOW_SPEED_MODE;
    fan_timer.duty_resolution = LEDC_TIMER_12_BIT;
    fan_timer.timer_num = LEDC_TIMER_0;
    fan_timer.freq_hz = freqHz;
    fan_timer.clk_cfg = LEDC_USE_APB_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&fan_timer));

    ledc_channel_config_t fan_channel = {};
    fan_channel.gpio_num = gpioNum;
    fan_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    fan_channel.channel = LEDC_CHANNEL_0;
    fan_channel.intr_type = LEDC_INTR_DISABLE;
    fan_channel.timer_sel = LEDC_TIMER_0;
    fan_channel.duty = duty;

    ESP_ERROR_CHECK(ledc_channel_config(&fan_channel));
}

void Fan::setGoIntensity(unsigned int goIntensity)
{
    m_accumGo = goIntensity;
    setSpeed();
}

void Fan::setStopIntensity(unsigned int stopIntensity)
{
    m_accumStop = stopIntensity;
    setSpeed();
}

void Fan::setSpeed()
{
    // Normalized intensity, 121 pixels in total, 8-bit per pixel
    float totalIntensity = (m_accumGo + m_accumStop) * 0.001080f;
    int fanIntensity = static_cast<int>(totalIntensity);
    if (fanIntensity >= 98) fanIntensity = 100; // might as well..

    unsigned int duty = fanIntensity / 100.0 * (1 << 12);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}

Fan fanControl;