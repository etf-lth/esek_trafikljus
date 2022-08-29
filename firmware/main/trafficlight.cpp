#include "trafficlight.h"
#include "spi.h"

#include <iostream>

#include "driver/gpio.h"

/* A global Traffic Light variable, since we need to access this from multiples places. */
TrafficLight g_trafficLight;

TrafficLight::TrafficLight() : m_stopColor(UniColor("ff0000")), m_goColor(UniColor("00ff00"))
{
	gpio_config_t io_conf{};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = (1ULL << GPIO_NUM_18);
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config(&io_conf);
}


void TrafficLight::setStopColor(const Color &stop)
{
	m_stopColor = stop;

	// Start frame
	stop_data[0] = 0x00;
	stop_data[1] = 0x00;
	stop_data[2] = 0x00;
	stop_data[3] = 0x00;

	// End frame
	stop_data[(STOP_LED_NUMBER+1)*4 + 0] = 0xff;
	stop_data[(STOP_LED_NUMBER+1)*4 + 1] = 0xff;
	stop_data[(STOP_LED_NUMBER+1)*4 + 2] = 0xff;
	stop_data[(STOP_LED_NUMBER+1)*4 + 3] = 0xff;

	for (int led = 1; led <= STOP_LED_NUMBER; led++) {
		Color::led_color(stop_data, led, m_stopColor.r(), m_stopColor.g(), m_stopColor.b());	// rgb
	}
		
	ESP_ERROR_CHECK(spi_device_queue_trans(spi_handle_stop, &trans_desc_stop, portMAX_DELAY));
	//vTaskDelay(10000 / portTICK_RATE_MS);
}


void TrafficLight::setGoColor(const Color &go)
{
	m_goColor = go;

}
