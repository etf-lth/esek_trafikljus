#pragma once

#include <string>

/**
 * TODO: Make an interface with iterators to get colors per pixel or per line
 */
class Color
{
private:
	unsigned int m_red;
	unsigned int m_green;
	unsigned int m_blue;

protected:
	Color(unsigned int red, unsigned int green, unsigned int blue);

	/**
	 * @brief Creates a color object from a hex string
	 *
	 * @param[in] hexCode	A hex string in the form "rrggbb".
	 */
	explicit Color(const std::string &hexCode);

public:
	unsigned int r() const { return m_red; }
	unsigned int g() const { return m_green; }
	unsigned int b() const { return m_blue; }
};


class UniColor : public Color
{
public:
	UniColor(unsigned int red, unsigned int green, unsigned int blue);
	explicit UniColor(const std::string &hexCode);
};
