#include "BikeSystem.h"

volatile bool BikeSystem::buttonPressed = false;

BikeSystem::BikeSystem(uint8_t redSensor, uint8_t magnetPin, uint8_t buttonPin, uint8_t buzzerPin, uint8_t lcdAddr)
    : redSensor(redSensor), magnetPin(magnetPin), buttonPin(buttonPin), buzzerPin(buzzerPin),
      lcd(lcdAddr), bikeTaken(false), biketa(false), returnbike(true),
      beepDuration(100), pauseDuration(50) {}

void BikeSystem::begin() {
    pinMode(redSensor, INPUT);
    pinMode(magnetPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    lcd.begin(16, 2);
    lcd.setBacklight(255);
    lcd.clear();
    delay(500);
    attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
}

void BikeSystem::run() {
    returnbike = true;
    borrowBike();
}

void BikeSystem::rentLCD() {
    lcd.clear();
    delay(1000);
    lcd.setCursor(5, 0);
    lcd.print("Enjoy");
    lcd.setCursor(3, 1);
    lcd.print("Your  Bike");
    delay(1000);
}

void BikeSystem::takeOut() {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Please");
    lcd.setCursor(3, 1);
    lcd.print("take  out");
    delay(1000);
    lcd.clear();
}

void BikeSystem::borrowBike() {
    if (digitalRead(buttonPin) == HIGH) {
        digitalWrite(magnetPin, HIGH);
        for (int i = 0; i < 4; i++) rentLCD();
        lcd.clear();
        startTime = millis();
        biketa = true;
        bikeTaken = false;
    }

    while (biketa) {
        if (buttonPressed) {
            buttonPressed = false;
            return;
        }

        if (digitalRead(redSensor) == HIGH) {
            digitalWrite(magnetPin, LOW);
            bikeTaken = true;
            biketa = false;
            returnBike();
        }

        if (millis() - startTime >= 4000) {
            if (!bikeTaken) {
                takeOut();
                digitalWrite(buzzerPin, HIGH); delay(beepDuration); digitalWrite(buzzerPin, LOW); delay(pauseDuration);
                digitalWrite(buzzerPin, HIGH); delay(beepDuration / 2); digitalWrite(buzzerPin, LOW); delay(beepDuration / 2);
                digitalWrite(buzzerPin, HIGH); delay(beepDuration / 2); digitalWrite(buzzerPin, LOW); delay(pauseDuration);
                digitalWrite(buzzerPin, HIGH); delay(beepDuration * 2); digitalWrite(buzzerPin, LOW); delay(pauseDuration);
            }
            if (bikeTaken) biketa = false;
        }
        delay(500);
    }
}

void BikeSystem::returnBike() {
    lcd.clear();
    delay(1000);
    lcd.setCursor(3, 0);
    lcd.print("Please");
    lcd.setCursor(1, 1);
    lcd.print("Return Bike");
    delay(2000);

    while (returnbike) {
        if (buttonPressed) {
            buttonPressed = false;
            return;
        }

        if (digitalRead(redSensor) == LOW) {
            lcd.clear();
            lcd.setCursor(3, 0);
            lcd.print("Detecting");
            lcd.setCursor(1, 1);
            lcd.print("State Of Charge");
            delay(4000);

            if (buttonPressed) {
                buttonPressed = false;
                return;
            }

            while (digitalRead(buttonPin) == LOW) {
                lcd.clear();
                delay(1000);
                lcd.setCursor(3, 0);
                lcd.print("Discharge...");
                delay(1000);
            }
            borrowBike();
        }
    }
}

void BikeSystem::buttonISR() {
    buttonPressed = true;
}
