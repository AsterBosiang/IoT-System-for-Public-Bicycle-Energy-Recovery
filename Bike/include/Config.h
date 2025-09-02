#pragma once
#include <Arduino.h>

// Hardware pins
constexpr int CS_PIN = 5;
constexpr int HALL_SENSOR_PIN = 14;
constexpr int RELAY_PIN = 25;
constexpr int LED_PIN = 13;
constexpr int BUTTON_PIN = 2;

// Wheel parameters
constexpr float WHEEL_DIAMETER = 0.64f;
constexpr float WHEEL_CIRCUMFERENCE = WHEEL_DIAMETER * PI;
constexpr int TEST_DURATION = 2; // seconds

// WiFi credentials
constexpr const char* WIFI_SSID = "";
constexpr const char* WIFI_PASSWORD = "";

// Server
constexpr const char* SERVER_HOST = "184.72.133.61";
constexpr const char* SERVER_UPLOAD_PATH = "/upload.php";

// Others
constexpr const char* DATA_FILE = "/example.txt";
