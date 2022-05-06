#include "trafficlight.h"

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
	if (stop.r() == 255)
	{
		std::cout << "TOP ON" << std::endl;
		gpio_set_level(GPIO_NUM_18, 1);
	}
	else if (stop.r() == 0)
	{
		std::cout << "TOP OFF" << std::endl;
		gpio_set_level(GPIO_NUM_18, 0);
	}
}


void TrafficLight::setGoColor(const Color &go)
{
	m_goColor = go;
}
