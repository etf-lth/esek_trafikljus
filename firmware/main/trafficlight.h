#pragma once

#include "color.h"

class TrafficLight
{
private:
	Color m_stopColor;
	Color m_goColor;

public:
	TrafficLight();

	void setStopColor(const Color &stop);
	void setGoColor(const Color &go);

	const Color& getStopColor() const { return m_stopColor; }
	const Color& getGoColor() const { return m_stopColor; }
};

extern TrafficLight g_trafficLight;
