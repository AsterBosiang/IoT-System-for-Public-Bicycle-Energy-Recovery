#pragma once
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Arduino.h>

class TimeManager {
public:
    TimeManager();
    void begin();
    void update();
    String getFormattedTime();

private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
    String formattedTime;
};
