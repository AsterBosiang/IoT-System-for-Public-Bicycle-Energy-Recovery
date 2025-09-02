#include "SensorManager.h"
#include "Config.h"

SensorManager::SensorManager(int hallPin)
    : hallPin(hallPin) {}

void SensorManager::begin() {
    pinMode(hallPin, INPUT);
    if (!ina219.begin()) {
        Serial.println("Failed to find INA219 chip");
    }
}

float SensorManager::measureSpeed() {
    unsigned long startTime = millis();
    int pulseCount = 0;

    while (millis() - startTime < TEST_DURATION * 1000) {
        if (digitalRead(hallPin) == HIGH) {
            while (digitalRead(hallPin) == HIGH); // avoid double count
            pulseCount++;
        }
    }

    float distance = pulseCount * WHEEL_CIRCUMFERENCE;
    return (distance / TEST_DURATION) * 3.6f; // km/h
}

float SensorManager::readVoltage() {
    return ina219.getBusVoltage_V();
}

float SensorManager::readCurrent() {
    return ina219.getCurrent_mA() / 1000.0f; // A
}

float SensorManager::readPower() {
    return ina219.getPower_mW() / 1000.0f; // W
}
