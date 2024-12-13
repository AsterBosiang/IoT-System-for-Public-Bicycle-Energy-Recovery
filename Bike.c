#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <WiFi.h>
#include <time.h>
#include "WiFiUdp.h" // not needed for WiFiNINA
#include "NTP.h"
#include <HTTPClient.h>
#include <Adafruit_INA219.h> // Add this line to include INA219 library

// SD card pin selection
#define CS_PIN 5
#define Hall_Sensor_D 14 // Hall sensor, digital output connected to ESP32 pin 14
#define WHEEL_DIAMETER 0.64 // Wheel diameter (in meters)
float wheelCircumference = WHEEL_DIAMETER * PI; // Wheel circumference
const int testDuration = 2; // Test duration, in seconds

ThreeWire myWire(15, 16, 4);
const int relayPin = 25;         // Relay pin
RtcDS1302 <ThreeWire> Rtc(myWire);
unsigned long timen = 0;  // Record time when Hall sensor receives signal
String bike_id = "1";
String serverURL;
int value = 0;
int led=13;                    // LED long leg connected to Arduino pin 13
int Val1=0;                    // Set integer Val1 and Val2
int Val2=0;  
int n;
int ROUT = 20;                // External resistance
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;;
float Amp = 0.0;
float Power = 0.0;          
float Radius = 32.198;
float time_gap = 0;      // Time interval received by sensor
float time_tem = 0;
float timef = 0;        // Convert recorded time to seconds
float rpm = 0;
float speed = 0;
String URL;
String URLDateTime;
bool display = 0;
const int buttonPin = 2;
bool buttonState = LOW;  // Default button is low level (open circuit)
bool isRepeating = false;  // Track whether repeating execution is occurring
bool hasExecutedOnce = false;  // Track whether it has been executed once
bool activatedornot = false;
bool lastactivatedornot = false;
int relayactivenumber = 0;
bool isRiding = false;
bool relayActive = false;
unsigned long lastTime = 0; // Last calculation time
bool lastState = LOW; // Previous sensor state

const char *ssid     = "Wi-Fi冷點"; // your network SSID
const char *password = "az1by2cx"; // your network PW
WiFiUDP wifiUdp;
NTP ntp(wifiUdp);
Adafruit_INA219 ina219;

void setup() {
  Serial.begin(9600);
  Serial.println("CLEARSHEET"); // clears sheet starting at row 1
  Serial.println("LABEL,Timer,vIN,Amp,Power,speed");
  pinMode(Hall_Sensor_D,INPUT);
  pinMode (led, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(relayPin, INPUT);
  pinMode(35, OUTPUT);
  digitalWrite(35, LOW);
    // Set I²C SDA and SCL pins
  pinMode(Hall_Sensor_D, INPUT);
  Wire.begin(21, 22);

   if (!SD.begin(CS_PIN)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  pinMode(buttonPin, INPUT_PULLUP);  // Set buttonPin as input and enable internal pull-up resistor

  // Check button state when initializing
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    // If button is initially open circuit (low level), execute initialization code and start repeating
    isRiding = true;
    Serial.println("Ride started");
    ConectWifi();
    ReachAndSyncTime();
    isRepeating = true;
  }
}

void loop() {
  buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH) {
    // Button receives high level, standby until button is released
    if (isRepeating) {
      // After completing repeated execution, execute code when button receives high level
      isRiding = false;
      ConectWifi();
      ReadAndUpload();
      isRepeating = false;
    }
    // Standby while button remains high
  } else {
    // Button open circuit (low level)
    if (!isRepeating) {
      // If previously in standby state, execute initialization code and start repeating
      isRiding = true;
      Serial.println("Ride started");
      ConectWifi();
      ReachAndSyncTime();
      isRepeating = true;
    }
    // Repeated execution code
        RtcDateTime now = Rtc.GetDateTime();
        URLDateTime = formatDateTime(now);
        if (!now.IsValid()) {
         Serial.println("RTC lost confidence in the DateTime!");
        }
        speed = measureSpeed(); // Call speed measurement subroutine
        if(speed>255) {
          speed=0;
        }
        constructURL();
        delay(10000);
        UploadSD();
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
    // Check if DS1302 is properly connected
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

String formatDateTime(const RtcDateTime& dt)
{
    char datestring[20];
    snprintf_P(datestring,
               countof(datestring),
               PSTR("%04u-%02u-%02uT%02u:%02u:%02u"),
               dt.Year(),
               dt.Month(),
               dt.Day(),
               dt.Hour(),
               dt.Minute(),
               dt.Second());

    return String(datestring);
}

float measureSpeed() {
    unsigned long startTime = millis(); // Start time
    int pulseCount = 0;                 // Pulse count

    // Calculate pulse count within test duration
    while (millis() - startTime < testDuration * 1000) {
        if (digitalRead(Hall_Sensor_D) == HIGH) {
            while (digitalRead(Hall_Sensor_D) == HIGH); // Wait for LOW to avoid repeated counting
            pulseCount++;
        }
    }

    // Calculate speed
    float distance = pulseCount * wheelCircumference; // Total travel distance, in meters
    float speedin = (distance / testDuration) * 3.6;    // Convert to km/h

    return speedin;
}

void constructURL(){
  String speedS = String(speed);
  String voltageS =  String(vIN);
  String currentS =  String(Amp);
  String powerS = String(Power);
  URL = "http://184.72.133.61/upload.php?bike_id=" + bike_id + 
               "&time=" + URLDateTime + 
               "&speed=" + speedS +
               "&voltage=" + voltageS + 
               "&current=" + currentS + 
               "&power=" + powerS;
}
void UploadSD() {
  // Open file and append data
  File file = SD.open("/example.txt", FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  Serial.println(URL);
  file.println(URL); // Write URL to a new line
  file.close();
  Serial.println("File written");
  delay(1000);
}

void ReadAndUpload() {
  File file = SD.open("/example.txt");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("File Content:");

  bool allDataUploaded = true; // Track if all data is uploaded
  while (file.available()) {
    serverURL = file.readStringUntil('\n');
    serverURL.trim();  // Clear spaces and newline characters before and after URL
    bool uploadSuccess = false;
    Serial.println(serverURL);

    while (!uploadSuccess) { // Keep trying until upload is successful
      HTTPClient http;
      http.begin(serverURL);
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println("Response:");
        Serial.println(response);
        uploadSuccess = true; // Mark as successful upload
      } else {
        Serial.print("HTTP GET request failed: ");
        Serial.println(httpResponseCode);
        uploadSuccess = false;
        allDataUploaded = false; // If any fail, we will keep the file for retrying
      }
      http.end();

      if (!uploadSuccess) {
        delay(5000); // Wait before retrying to avoid spamming the server
      }
    }
  }
  file.close();

  // If all data was uploaded successfully, delete the file
  if (allDataUploaded) {
    if (SD.remove("/example.txt")) {
      Serial.println("File deleted successfully");
    } else {
      Serial.println("Failed to delete file");
    }
  } else {
    Serial.println("Not all data was uploaded. Retrying later.");
  }
}
