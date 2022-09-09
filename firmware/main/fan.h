
#pragma once

class Fan
{
public:
    static void init();
    /**
     * Set the speed (duty cycle) between 0 - 100
     */
    static void setSpeed(unsigned int val);
};