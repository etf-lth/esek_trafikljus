#include "trafficlight.h"


TrafficLight::TrafficLight(const Color &top, const Color &bottom) : m_topColor(top), m_bottomColor(bottom)
{
}


void TrafficLight::setTopColor(const Color &top)
{
}


void TrafficLight::setBottomColor(const Color &bottom)
{
}

TrafficLight g_trafficLight(UniColor("ff0000"), UniColor("00ff00"));