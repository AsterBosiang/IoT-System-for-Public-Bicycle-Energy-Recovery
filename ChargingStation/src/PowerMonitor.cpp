#include "PowerMonitor.h"
#include <Arduino.h>

PowerMonitor::PowerMonitor() : busVoltage(0.0f), current_mA(0.0f) {}

bool PowerMonitor::begin() {
    return ina219.begin();
}

void PowerMonitor::measureOCV() {
    int value = analogRead(Config::VOLTAGE_SENSOR_PIN);
    float vOut = (value * 5.0f) / 1024.0f;
    busVoltage = vOut / (Config::R2 / (Config::R1 + Config::R2));
}

void PowerMonitor::measureBusVoltage() {
    measureOCV();
}

void PowerMonitor::measureCurrent() {
    current_mA = ina219.getCurrent_mA();
}
