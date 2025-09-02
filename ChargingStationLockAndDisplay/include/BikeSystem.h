#pragma once
#include <Arduino.h>
#include <LiquidCrystal_PCF8574.h>

class BikeSystem {
public:
    BikeSystem(uint8_t redSensor, uint8_t magnetPin, uint8_t buttonPin, uint8_t buzzerPin, uint8_t lcdAddr);
    void begin();
    void run();

private:
    void rentLCD();
    void takeOut();
    void borrowBike();
    void returnBike();
    static void buttonISR();

    static volatile bool buttonPressed;
    bool bikeTaken;
    bool biketa;
    bool returnbike;
    unsigned long startTime;

    uint8_t redSensor;
    uint8_t magnetPin;
    uint8_t buttonPin;
    uint8_t buzzerPin;

    int beepDuration;
    int pauseDuration;

    LiquidCrystal_PCF8574 lcd;
};
