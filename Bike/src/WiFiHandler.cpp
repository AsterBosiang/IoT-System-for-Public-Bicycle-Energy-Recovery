#include "WiFiHandler.h"

void WiFiHandler::connect(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

bool WiFiHandler::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}
