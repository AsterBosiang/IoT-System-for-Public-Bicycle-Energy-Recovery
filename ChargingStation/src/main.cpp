#include <Arduino.h>
#include "Config.h"
#include "PowerMonitor.h"
#include "TimeManager.h"
#include "NetworkManager.h"

PowerMonitor powerMonitor;
TimeManager timeManager;
NetworkManager network;

void setup() {
    Serial.begin(9600);
    pinMode(Config::BUTTON_PIN, INPUT_PULLUP);
    pinMode(Config::RELAY_PIN, OUTPUT);

    if (!powerMonitor.begin()) {
        Serial.println("Failed to init INA219");
        while (true) delay(100);
    }

    network.connectWiFi();
    timeManager.begin();
}

void loop() {
    int buttonState = digitalRead(Config::BUTTON_PIN);
    if (buttonState == LOW) {
        powerMonitor.measureBusVoltage();
        powerMonitor.measureCurrent();

        String url = "/charging_station_upload.php?bike_id=" + String(Config::BIKE_ID) +
                     "&time=" + timeManager.getFormattedTime() +
                     "&voltage=" + String(powerMonitor.getVoltage()) +
                     "&current=" + String(powerMonitor.getCurrent()) +
                     "&power=" + String(powerMonitor.getPower());

        network.sendData(url);
    }

    timeManager.update();
    delay(1000);
}
