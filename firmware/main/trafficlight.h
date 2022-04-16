#pragma once

#include "color.h"

class TrafficLight
{
private:
	Color m_topColor;
	Color m_bottomColor;

public:
	TrafficLight();

	void setTopColor(const Color &top);
	void setBottomColor(const Color &bottom);

	const Color& getTopColor() const { return m_topColor; }
};

extern TrafficLight g_trafficLight;
