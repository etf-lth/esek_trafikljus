#pragma once

#include "color.h"

class TrafficLight
{
private:
	Color m_topColor;
	Color m_bottomColor;

public:
	TrafficLight(const Color &top, const Color &bottom);

	void setTopColor(const Color &top);
	void setBottomColor(const Color &bottom);
};

extern TrafficLight g_trafficLight;