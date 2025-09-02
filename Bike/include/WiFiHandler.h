#pragma once
#include <WiFi.h>

class WiFiHandler {
public:
    void connect(const char* ssid, const char* password);
    bool isConnected() const;
};
