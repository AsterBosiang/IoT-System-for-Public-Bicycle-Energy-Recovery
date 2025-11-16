#pragma once
#include <Arduino.h>
#include <Adafruit_INA219.h>

class SensorManager {
public:
    SensorManager(int hallPin);
    void begin();
    float measureSpeed();
    float readVoltage();
    float readCurrent();
    float readPower();
private:
    int hallPin;
    Adafruit_INA219 ina219;
};
