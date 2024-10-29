#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <WiFi.h>
#include <time.h>
#include "WiFiUdp.h" // Not needed for WiFiNINA
#include "NTP.h"
#include <HTTPClient.h>
#include <Adafruit_INA219.h> // Include INA219 library

// SD card select pin
#define CS_PIN 5
#define Hall_Sensor_D 14 // Hall sensor digital output connected to ESP32 pin 14

ThreeWire myWire(15, 16, 4);
const int relayPin = 25;         // Relay pin
RtcDS1302 <ThreeWire> Rtc(myWire);
unsigned long timen = 0;  // Records the time when the Hall sensor receives a signal
String bike_id = "1";
String serverURL;
int value = 0;
int led=13;                    // LED long leg connected to Arduino pin 13
int Val1=0;                    // Set integer Val1 and Val2
int Val2=0;  
int n;
int ROUT = 20;                // External resistor
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
float Amp = 0.0;
float Power = 0.0;          
float Radius = 32.198;
float time_gap = 0;      // Time interval when sensor receives signal
float time_tem = 0;
float timef = 0;        // Converts recorded time to seconds
float rpm = 0;
float speed = 0;
String URL;
String URLDateTime;
bool display = 0;
const int buttonPin = 2;
bool buttonState = LOW;  // Default button state is low (not connected)
bool isRepeating = false;  // Track if repeating is in progress
bool hasExecutedOnce = false;  // Track if executed once
bool activatedornot = false;
bool lastactivatedornot = false;
int relayactivenumber = 0;
unsigned long rideStartTime = 0;
unsigned long rideEndTime = 0;
unsigned long rideTime = 0;
unsigned long relayStartTime = 0;
unsigned long relayDuration = 0;
unsigned long distance = 0;
const float wheelCircumference = 2 * PI * 32.198; // Wheel circumference
bool isRiding = false;
bool relayActive = false;
unsigned long startTime = 0; // Start time for calculations
unsigned long windowTime = 1000; // Time window in ms, set to 1 second
int rotationCount = 0; // Rotation count
bool lastState = LOW; // Previous sensor state


const char *ssid     = "Wi-Fi Hotspot"; // Network SSID
const char *password = "az1by2cx"; // Network Password
WiFiUDP wifiUdp;
NTP ntp(wifiUdp);
Adafruit_INA219 ina219;

void setup() {
  Serial.begin(9600);
  Serial.println("CLEARSHEET"); // clears sheet starting at row 1
  Serial.println("LABEL,Timer,vIN,Amp,Power,speed");
  pinMode(Hall_Sensor_D, INPUT);
  pinMode(led, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(relayPin, INPUT);
  pinMode(35, OUTPUT);
  digitalWrite(35, LOW);

  // Set I²C SDA and SCL pins
  Wire.begin(21, 22);

  // Initialize INA219
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1);
  }

  Serial.println("INA219 setup success!");

  if (!SD.begin(CS_PIN)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  pinMode(buttonPin, INPUT_PULLUP);  // Set buttonPin as input with pull-up resistor

  // Check button state at initialization
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    // If button is not connected initially (low level), run initial code once and start repeating
    isRiding = true;
    rideStartTime = millis();
    Serial.println("Ride started");
    ConectWifi();
    ReachAndSyncTime();
    isRepeating = true;
  }
}

void loop() {
  buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH) {
    // Button receives high signal, wait until button is released
    if (isRepeating) {
      // After completing repeating, execute button high signal code once
      isRiding = false;
      rideEndTime = millis();
      rideTime = (rideEndTime - rideStartTime) / 1000;
      ConectWifi();
      ReadAndUpload();
      uploadfour();
      isRepeating = false;
    }
    // Program waits while button is in high signal state
  } else {
    // Button is not connected (low signal)
    if (!isRepeating) {
      // If previously in wait state, run initial code once and start repeating
      isRiding = true;
      rideStartTime = millis();
      Serial.println("Ride started");
      ConectWifi();
      ReachAndSyncTime();
      isRepeating = true;
    }
    // Repeating code
    RtcDateTime now = Rtc.GetDateTime();
    URLDateTime = formatDateTime(now);
    if (!now.IsValid()) {
      Serial.println("RTC lost confidence in the DateTime!");
    }
    Sensor();
    float currentSpeed = measureSpeed(); // Call subroutine to get speed
    speed = currentSpeed;
    constructURL();
    UploadSD();
    if (relayActive) {
      activatedornot = true;
      relayDuration += millis() - relayStartTime;
      relayStartTime = millis();  // Update relay activation start time
      lastactivatedornot = true;
    } else {
      activatedornot = false;
      if (activatedornot != lastactivatedornot) {
        relayactivenumber++;
      }
      lastactivatedornot = false;
    }
  }
}

void ConectWifi(){
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void ReachAndSyncTime(){
  ntp.ruleSTD("CST", Last, Sun, Mar, 2, 480);
  ntp.begin();
  Serial.println("start NTP");
  ntp.update();
  // Print formatted time 2024-04-19T12:34:56
  Serial.println(ntp.formattedTime("%Y-%m-%dT%H:%M:%S"));
  
  Rtc.Begin();
  // Check if DS1302 is correctly connected
  if (!Rtc.IsDateTimeValid()) {
    Serial.println("RTC lost confidence in the DateTime!");
  }
  if (Rtc.GetIsWriteProtected()) {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  // Set DS1302 time using NTP time
  String formattedTime = ntp.formattedTime("%Y-%m-%dT%H:%M:%S");
  Serial.println("NTP time: " + formattedTime);

  // Parse time string from NTP
  int year = formattedTime.substring(0, 4).toInt();
  int month = formattedTime.substring(5, 7).toInt();
  int day = formattedTime.substring(8, 10).toInt();
  int hour = formattedTime.substring(11, 13).toInt();
  int minute = formattedTime.substring(14, 16).toInt();
  int second = formattedTime.substring(17, 19).toInt();

  // Set DS1302 time
  RtcDateTime ntpTime(year, month, day, hour, minute, second);
  Rtc.SetDateTime(ntpTime);
  Serial.println("RTC time updated with NTP time");
}
// Format the date and time from RtcDS1302
String formatDateTime(const RtcDateTime& dt) {
  char datestring[20];
  snprintf_P(datestring, 
             sizeof(datestring),
             PSTR("%04u-%02u-%02uT%02u:%02u:%02u"),
             dt.Year(),
             dt.Month(),
             dt.Day(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  return datestring;
}

// Construct the URL for data upload
void constructURL() {
  URL = "http://example.com/upload?bike_id=" + bike_id;
  URL += "&time=" + URLDateTime;
  URL += "&vin=" + String(vIN, 2);
  URL += "&current=" + String(Amp, 2);
  URL += "&power=" + String(Power, 2);
  URL += "&speed=" + String(speed, 2);
}

// Upload data stored in the SD card
void UploadSD() {
  File file = SD.open("/data.txt");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  while (file.available()) {
    String line = file.readStringUntil('\n');
    Serial.println("Uploading: " + line);
    // Implement actual data upload here if needed
  }
  file.close();
}

// Upload data to the server
void ReadAndUpload() {
  constructURL();
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(URL);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

// Calculate speed based on the number of rotations within a fixed time window
float measureSpeed() {
  unsigned long currentTime = millis();
  if (currentTime - startTime >= windowTime) {
    float rotationsPerSecond = rotationCount / (windowTime / 1000.0);
    float speed = rotationsPerSecond * wheelCircumference;
    startTime = currentTime;
    rotationCount = 0;
    return speed;
  }
  return speed;
}

// Main sensor function that reads data from sensors
void Sensor() {
  Val1 = digitalRead(Hall_Sensor_D); // Read the Hall sensor state
  if (Val1 != Val2) { // Check if the state has changed
    if (Val1 == HIGH) { // If sensor detects a signal
      rotationCount++;
      time_gap = (millis() - timen) / 1000.0; // Calculate time gap in seconds
      timen = millis(); // Update the last signal time
    }
    Val2 = Val1; // Update last sensor state
  }
  vOUT = analogRead(35) * (3.3 / 4096.0); // Read and scale analog input
  vIN = vOUT * ((R1 + R2) / R2); // Calculate input voltage based on resistor divider
  Amp = ina219.getCurrent_mA() / 1000.0; // Get current in amps
  Power = vIN * Amp; // Calculate power in watts
}

// Upload additional data to server
void uploadfour() {
  String extraURL = "http://example.com/upload/additional_data?bike_id=" + bike_id;
  extraURL += "&relay_count=" + String(relayactivenumber);
  extraURL += "&relay_time=" + String(relayDuration / 1000.0);
  extraURL += "&distance=" + String(distance);
  extraURL += "&riding_time=" + String(rideTime);
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(extraURL);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Additional data upload response: " + response);
    } else {
      Serial.println("Error on HTTP request for additional data");
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
