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


UniColor::UniColor(unsigned int red, unsigned int green, unsigned int blue) : Color(red, green, blue)
{
}


UniColor::UniColor(const std::string &hexCode) : Color(hexCode)
{
}
