#pragma once
#include <WiFi.h>
#include "Config.h"

class NetworkManager {
public:
    void connectWiFi();
    bool sendData(const String& url);
};
