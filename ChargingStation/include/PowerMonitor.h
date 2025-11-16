#pragma once
#include <Adafruit_INA219.h>
#include "Config.h"

class PowerMonitor {
public:
    PowerMonitor();
    bool begin();
    void measureOCV();
    void measureBusVoltage();
    void measureCurrent();

    float getVoltage() const { return busVoltage; }
    float getCurrent() const { return current_mA; }
    float getPower() const { return busVoltage * current_mA; }

private:
    Adafruit_INA219 ina219;
    float busVoltage;
    float current_mA;
};
