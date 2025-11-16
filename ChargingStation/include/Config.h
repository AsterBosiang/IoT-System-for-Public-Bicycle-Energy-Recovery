#pragma once
#include <Arduino.h>

namespace Config {
    // WiFi
    constexpr const char* WIFI_SSID = "";
    constexpr const char* WIFI_PASSWORD = "";

    // Server
    constexpr const char* SERVER_IP = "184.72.133.61";
    constexpr int SERVER_PORT = 80;

    // Sensors & Pins
    constexpr int VOLTAGE_SENSOR_PIN = A0;
    constexpr int BUTTON_PIN = 5;
    constexpr int RELAY_PIN = 9;
    constexpr int RESET_PIN = 4;

    // Voltage Divider
    constexpr float R1 = 30000.0f;
    constexpr float R2 = 7500.0f;

    // Bike ID
    constexpr const char* BIKE_ID = "1";
}
