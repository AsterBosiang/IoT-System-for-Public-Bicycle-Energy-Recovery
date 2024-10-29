#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <NTPClient.h>

const char* ssid = "Wi-Fi冷點";
const char* password = "az1by2cx";
const char* server = "184.72.133.61";  // 伺服器IP地址
const int port = 80;  // HTTP端口

const int voltageSensor = A0;
Adafruit_INA219 ina219;
float current_mA = 0;
int value = 0;
float vOUT = 0.0;
float vIN = 0.0;
float busvoltage = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
String bike_id = "1";
String voltage = "24.7";
String current = "2.3";
String power = "56.7";
const int buttonPin = 5;
int buttonState = 5;   // 按鈕狀態
String url;
String formattedTime;
int status = WL_IDLE_STATUS; // 狀態變數
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 8 * 3600, 60000); // 設置 NTP 客戶端，UTC+8

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  // 使用內建上拉電阻
  Serial.begin(9600);
  while (!Serial) 
  {
    delay(1);
  }
  if (!ina219.begin())
  {
    Serial.println("Failed to find INA219 chip");
    while (1) 
    {
      delay(10);
    }
  }
  pinMode(9, OUTPUT); //繼電器開關
  ocv();
  // 連接到WiFi網路
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
    // 啟動 NTP 客戶端
  timeClient.begin();
}

void loop() {
  buttonState = digitalRead(buttonPin); // 讀取按鈕狀態
  delay(1000);
  while (digitalRead(buttonPin) == LOW) {
    if (vIN>3.3) //ocv判斷標準
  {
    digitalWrite(9,HIGH);
    measure();
    Serial.println("Charging");
    if (vIN>3.15)
    {
      busvolatage();
      currentmeasure();
    }
    else
    {
      digitalWrite(9,LOW);
      busvolatage();
      Serial.println("battery is not full enough");
      detectleaveornot();
    }
  }
  else
  {
    Serial.println("battery is not full enough");
    detectleaveornot();
  }
    // 更新格式化日期和時間
  updateFormattedTime(timeClient);
  constructeURL();
  sendit();


  }


}

void resetESP8266() {
  Serial.println("Resetting ESP8266...");
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);  // 將重置引腳設為低電平
  delay(100);            // 保持低電平一段時間
  digitalWrite(4, HIGH); // 恢復高電平
}

void ocv()
{
  value = analogRead(voltageSensor);
  vOUT = (value * 5.0) / 1024.0;
  vIN = vOUT / (R2/(R1+R2));
  delay(100);
}

void busvolatage()
{
  value = analogRead(voltageSensor);
  vOUT = (value * 5.0) / 1024.0;
  vIN = vOUT / (R2/(R1+R2));
  busvoltage = vIN;
  delay(100);
}

void measure()
{
  value = analogRead(voltageSensor);
  vOUT = (value * 5.0) / 1024.0;
  vIN = vOUT / (R2/(R1+R2));
}

void currentmeasure()
{
  current_mA = ina219.getCurrent_mA();
  delay(100);
}

void updateFormattedTime(NTPClient &timeClient) {
  timeClient.update();

  // 獲取時間戳並計算日期和時間
  unsigned long rawTime = timeClient.getEpochTime();
  unsigned long days = rawTime / 86400L;
  unsigned long years = 1970;
  unsigned long months, date, hours, minutes, seconds;

  // 計算年份
  while ((days >= 365 && (years % 4 != 0 || years % 100 == 0 && years % 400 != 0)) || (days >= 366 && (years % 4 == 0 && (years % 100 != 0 || years % 400 == 0)))) {
    if (years % 4 == 0 && (years % 100 != 0 ||years % 400 == 0)) {
      days -= 366;
    } else {
      days -= 365;
    }
    years++;
  }

  // 計算月份和日期
  unsigned int daysInMonth[] = {31, (years % 4 == 0 && (years % 100 != 0 || years % 400 == 0)) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  months = 0;
  while (days >= daysInMonth[months]) {
    days -= daysInMonth[months];
    months++;
  }
  date = days + 1;
  months++;

  // 計算時間
  hours = (rawTime  % 86400L) / 3600;
  minutes = (rawTime % 3600) / 60;
  seconds = rawTime % 60;

  // 格式化日期和時間
  formattedTime = String(years) + "-" + 
                  (months < 10 ? "0" : "") + String(months) + "-" + 
                  (date < 10 ? "0" : "") + String(date) + "T" + 
                  (hours < 10 ? "0" : "") + String(hours) + ":" + 
                  (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
                  (seconds < 10 ? "0" : "") + String(seconds);
}

void constructeURL(){
  String voltage =  String(busvoltage);
  String current =  String(current_mA);
  float powerf = busvoltage*current_mA;
  String power = String(powerf);
  url = "/charging_station_upload.php?bike_id=" + bike_id + 
               "&time=" + formattedTime + 
               "&voltage=" + voltage + 
               "&current=" + current + 
               "&power=" + power;
  Serial.println(url);
}

void sendit(){
  WiFiClient client;
  if (client.connect(server, port)) {
    Serial.println("Connected to server");
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");

    // 讀取伺服器回應
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }
    client.stop();
    Serial.println("Disconnected from server");
  } else {
    Serial.println("Connection to server failed");
  }
}

void detectleaveornot(){
    buttonState = digitalRead(buttonPin); // 讀取按鈕狀態
    while (digitalRead(buttonPin) == LOW) {
    delay(1000);
    }
}