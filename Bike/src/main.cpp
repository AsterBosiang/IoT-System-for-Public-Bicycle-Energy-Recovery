#include <Arduino.h>
#include "Config.h"
#include "WiFiHandler.h"
#include "TimeManager.h"
#include "SensorManager.h"
#include "DataLogger.h"
#include "Uploader.h"

WiFiHandler wifi;
TimeManager timeMgr;
SensorManager sensor(HALL_SENSOR_PIN);
DataLogger logger;
Uploader uploader;

bool isRiding = false;

void setup() {
    Serial.begin(9600);

    pinMode(HALL_SENSOR_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);

    wifi.connect(WIFI_SSID, WIFI_PASSWORD);
    timeMgr.begin();
    timeMgr.syncWithNTP();
    sensor.begin();
    logger.begin(CS_PIN);
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW && !isRiding) {
        isRiding = true;
        Serial.println("Ride started");
    }

    if (isRiding) {
        float speed = sensor.measureSpeed();
        String now = timeMgr.nowString();

        String url = String("http://") + SERVER_HOST + SERVER_UPLOAD_PATH +
            "?bike_id=1&time=" + now +
            "&speed=" + speed;

        logger.log(url);
        uploader.upload(url);

        delay(10000);
    }
}
