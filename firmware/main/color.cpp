#include "color.h"


Color::Color(unsigned int red, unsigned int green, unsigned int blue) : m_red(red), m_green(green), m_blue(blue)
{
}


Color::Color(const std::string &hexCode)
{
	m_red = std::stoul(hexCode.substr(0, 2), nullptr, 16);
	m_green = std::stoul(hexCode.substr(2, 2), nullptr, 16);
	m_blue = std::stoul(hexCode.substr(4, 2), nullptr, 16);
}


uint32_t Color::hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v)
{
	uint8_t f = (h % 60) * 255 / 60;
	uint8_t p = (255 - s) * (uint16_t)v / 255;
	uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
	uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
	uint32_t r = 0, g = 0, b = 0;

	switch((h / 60) % 6){
		case 0: r = v; g = t; b = p; break;
		case 1: r = q; g = v; b = p; break;
		case 2: r = p; g = v; b = t; break;
		case 3: r = p; g = q; b = v; break;
		case 4: r = t; g = p; b = v; break;
		case 5: r = v; g = p; b = q; break;
	}
	return (r << 16 | g << 8 | b);
}


void Color::led_color(char *leds, uint8_t led_number, char r, char g, char b)
{
	leds[4*led_number + 0] = 0xff;  // TODO: allocate this in the beginning, it does not change
	leds[4*led_number + 1] = r;
	leds[4*led_number + 2] = b;
	leds[4*led_number + 3] = g;
}


/**
 * Unicolor class
 */
UniColor::UniColor(unsigned int red, unsigned int green, unsigned int blue) : Color(red, green, blue)
{
}


UniColor::UniColor(const std::string &hexCode) : Color(hexCode)
{
}
