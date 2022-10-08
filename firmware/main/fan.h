
#pragma once

#include "hal/ledc_types.h"

class Fan
{
public:
    /**
     * Initializes the fan controller to default values:
     *  - GPIO: 16
     *  - CLK: 10 kHz
     *  - Duty: 0
     */
    void init(int gpioNum = 16, uint32_t freqHz = 10000, uint32_t duty = 0);

    /**
     * Go intensity between 0 - 46665 (61 pixels)
     */
    void setGoIntensity(unsigned int goIntensity);

    /**
     * Stop intensity between 0 - 45900 (60 pixels)
     */
    void setStopIntensity(unsigned int stopIntensity);

private:
    unsigned int m_accumGo;
    unsigned int m_accumStop;

    /**
     * Sets the speed based on sign intensities (duty: 0-100)
     */
    void setSpeed();
};

extern Fan fanControl;