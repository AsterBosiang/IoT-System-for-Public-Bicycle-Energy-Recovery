#include "NetworkManager.h"
#include <WiFiClient.h>
#include <Arduino.h>

void NetworkManager::connectWiFi() {
    WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

bool NetworkManager::sendData(const String& url) {
    WiFiClient client;
    if (client.connect(Config::SERVER_IP, Config::SERVER_PORT)) {
        Serial.println("Connected to server");
        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: " + Config::SERVER_IP + "\r\n" +
                     "Connection: close\r\n\r\n");

        while (client.connected() || client.available()) {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                Serial.println(line);
            }
        }
        client.stop();
        return true;
    } else {
        Serial.println("Connection failed");
        return false;
    }
}
