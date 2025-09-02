#include "TimeManager.h"

TimeManager::TimeManager() 
    : wire(15, 16, 4), rtc(wire), ntp(udp) {}

void TimeManager::begin() {
    rtc.Begin();
    if (!rtc.GetIsRunning()) rtc.SetIsRunning(true);
}

void TimeManager::syncWithNTP() {
    ntp.ruleSTD("CST", Last, Sun, Mar, 2, 480);
    ntp.begin();
    ntp.update();

    String formattedTime = ntp.formattedTime("%Y-%m-%dT%H:%M:%S");
    int year = formattedTime.substring(0, 4).toInt();
    int month = formattedTime.substring(5, 7).toInt();
    int day = formattedTime.substring(8, 10).toInt();
    int hour = formattedTime.substring(11, 13).toInt();
    int minute = formattedTime.substring(14, 16).toInt();
    int second = formattedTime.substring(17, 19).toInt();

    RtcDateTime ntpTime(year, month, day, hour, minute, second);
    rtc.SetDateTime(ntpTime);

    Serial.println("RTC synced with NTP");
}

String TimeManager::nowString() {
    RtcDateTime now = rtc.GetDateTime();
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04u-%02u-%02uT%02u:%02u:%02u",
             now.Year(), now.Month(), now.Day(),
             now.Hour(), now.Minute(), now.Second());
    return String(buffer);
}
