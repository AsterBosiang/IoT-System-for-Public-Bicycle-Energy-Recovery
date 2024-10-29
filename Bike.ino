#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <WiFi.h>
#include <time.h>
#include "WiFiUdp.h"
#include "NTP.h"
#include <HTTPClient.h>
#include <Adafruit_INA219.h> // 加入這行來包含 INA219 庫

// SD卡片選擇引腳
#define CS_PIN 5
#define Hall_Sensor_D 14 // 霍爾感測器，數位輸出連結 ESP32 pin 14

ThreeWire myWire(15, 16, 4);
const int relayPin = 25;         // 繼電器腳位
RtcDS1302 <ThreeWire> Rtc(myWire);
unsigned long timen = 0;  //記錄霍爾感測器接收到訊號的時間
String bike_id = "1";
String serverURL;
int value = 0;
int led=13;                    //LED 長腳連接 Arduino pin 13
int Val1=0;                    //設定整數 Val1 and Val2
int Val2=0;  
int n;
int ROUT = 20;                //外部電阻
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;;
float Amp = 0.0;
float Power = 0.0;          
float Radius = 32.198;
float time_gap = 0;      //感測器接收到的時間間隔
float time_tem = 0;
float timef = 0;        //將紀錄的時間轉換為秒
float rpm = 0;
float speed = 0;
String URL;
String URLDateTime;
bool display = 0;
const int buttonPin = 2;
bool buttonState = LOW;  // 預設按鈕為低電平（空接）
bool isRepeating = false;  // 用來追蹤是否正在重複執行
bool hasExecutedOnce = false;  // 用來追蹤是否已執行過一次
bool activatedornot = false;
bool lastactivatedornot = false;
int relayactivenumber = 0;
unsigned long rideStartTime = 0;
unsigned long rideEndTime = 0;
unsigned long rideTime = 0;
unsigned long relayStartTime = 0;
unsigned long relayDuration = 0;
unsigned long distance = 0;
const float wheelCircumference = 2 * PI * 32.198; // 車輪圓周
bool isRiding = false;
bool relayActive = false;
unsigned long startTime = 0; // 計算開始時間
unsigned long windowTime = 1000; // 時間窗口（以毫秒為單位），這裡設置為1秒
int rotationCount = 0; // 旋轉次數
bool lastState = LOW; // 上一次的感測器狀態


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
    // 設定 I²C 的 SDA 和 SCL 引腳
  pinMode(Hall_Sensor_D, INPUT);
  Wire.begin(21, 22);
    // 初始化 INA219
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
  pinMode(buttonPin, INPUT_PULLUP);  // 設定 buttonPin 為輸入，並啟用內建上拉電阻

  // 初始化時檢查按鈕狀態
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    // 如果按鈕一開始是空接（低電平），執行一次初始程式碼並開始重複執行
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
    // 按鈕收到高電位，待命直到按鈕釋放
    if (isRepeating) {
      // 完成重複執行後，執行一次按鈕收到高電位時的程式碼
      isRiding = false;
      rideEndTime = millis();
      rideTime = (rideEndTime - rideStartTime)/1000;
      ConectWifi();
      ReadAndUpload();
      uploadfour();
      isRepeating = false;
    }
    // 按鈕保持高電位狀態時，程式待命
  } else {
    // 按鈕空接（低電位）
    if (!isRepeating) {
      // 如果先前處於待命狀態，執行一次初始程式碼並開始重複執行
      isRiding = true;
      rideStartTime = millis();
      Serial.println("Ride started");
      ConectWifi();
      ReachAndSyncTime();
      isRepeating = true;
    }
    // 重複執行的程式碼
        RtcDateTime now = Rtc.GetDateTime();
        URLDateTime = formatDateTime(now);
        if (!now.IsValid()) {
         Serial.println("RTC lost confidence in the DateTime!");
        }
        Sensor();
        float currentSpeed = measureSpeed(); // 呼叫副程式獲取轉速
        speed=currentSpeed;
        constructURL();
        UploadSD();
          if (relayActive) {
           activatedornot = true;
           relayDuration += millis() - relayStartTime;
           relayStartTime = millis();  // 更新繼電器導通開始時間
           lastactivatedornot = true;
          }else {
           activatedornot = false;
           if (activatedornot != lastactivatedornot){
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
// 打印格式化時間 2024-04-19T12:34:56
Serial.println(ntp.formattedTime("%Y-%m-%dT%H:%M:%S"));
    Rtc.Begin();
    // 判斷 DS1302 是否接好
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
    // 使用 NTP 時間設置 DS1302 時間
    String formattedTime = ntp.formattedTime("%Y-%m-%dT%H:%M:%S");
    Serial.println("NTP time: " + formattedTime);

    // 解析 NTP 獲取的時間字串
    int year = formattedTime.substring(0, 4).toInt();
    int month = formattedTime.substring(5, 7).toInt();
    int day = formattedTime.substring(8, 10).toInt();
    int hour = formattedTime.substring(11, 13).toInt();
    int minute = formattedTime.substring(14, 16).toInt();
    int second = formattedTime.substring(17, 19).toInt();

    // 設置 DS1302 時間
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

void Sensor(){
  float value = ina219.getBusVoltage_V();
  vOUT = (value * 5.0) / 1024.0;
  vIN = vOUT / (R2/(R1+R2));
  Amp = vIN / ROUT;
  Power = vIN * Amp;
}

float measureSpeed() {
  unsigned long currentTime = millis();
  float speed = 0; // 初始化速度

  // 如果當前時間超過時間窗口
  if (currentTime - startTime >= windowTime) {
    // 計算速度
    float rpm = (rotationCount * 60000.0) / windowTime; // 將旋轉次數轉換為RPM
    speed = rpm * 0.12138; // 將RPM轉換為速度（假設單位為km/h）

    // 重置計數和時間
    rotationCount = 0;
    startTime = currentTime;
  }

  // 讀取霍爾感測器的狀態
  if (digitalRead(Hall_Sensor_D) == HIGH && lastState == LOW) {
    rotationCount++; // 計算旋轉次數
  }

  lastState = digitalRead(Hall_Sensor_D); // 更新上一次的感測器狀態

  return speed; // 返回計算出的速度
}

void Record_Time(){
  timen = millis();
  digitalWrite(led, HIGH);
  timef = (timen / 1000.0);
  distance += wheelCircumference;  // 增加行駛距離
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
  // 打開文件並附加數據
  File file = SD.open("/example.txt", FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  Serial.println(URL);
  file.println(URL); // 將URL寫入新的一行
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

  bool allDataUploaded = true; // 確認資料是否上傳
  while (file.available()) {
    serverURL = file.readStringUntil('\n');
    serverURL.trim();  // 清除 URL 前后的空格和换行符
    bool uploadSuccess = false;
    Serial.println(serverURL);

    while (!uploadSuccess) { // 重複直到上傳成功
      HTTPClient http;
      http.begin(serverURL);
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println("Response:");
        Serial.println(response);
        uploadSuccess = true; // 記錄上傳成功
      } else {
        Serial.print("HTTP GET request failed: ");
        Serial.println(httpResponseCode);
        uploadSuccess = false;
        allDataUploaded = false; // 若失敗則保留資料
      }
      http.end();

      if (!uploadSuccess) {
        delay(5000); 
      }
    }
  }
  file.close();

  // 若所有資料皆上傳完成則刪除檔案
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

void uploadfour(){
  String distanceS = String(distance);
  String rideTimeS =  String(rideTime);
  String relayactivenumberS =  String(relayactivenumber);
  String relayDurationS = String(relayDuration);
  String URL = "http://184.72.133.61/data_analysis.php?relay_activations=" + relayactivenumberS + 
               "&relay_time=" + relayDurationS + 
               "&riding_distance=" + distanceS +
               "&riding_time=" + rideTimeS;
    HTTPClient http;
    // 開始 HTTP GET 請求
    http.begin(URL);
    // 發送請求並獲取狀態碼
    int httpResponseCode = http.GET();
    // 檢查響應
    if (httpResponseCode > 0) {
        // 請求成功
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        // 獲取響應
        String response = http.getString();
        Serial.println("Response:");
        Serial.println(response);
    } else {
        // 請求失敗
        Serial.print("HTTP GET request failed: ");
        Serial.println(httpResponseCode);
    }
    // 停止 HTTP 連接
    http.end();
}