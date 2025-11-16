#include "TimeManager.h"

TimeManager::TimeManager() 
    : timeClient(ntpUDP, "pool.ntp.org", 8 * 3600, 60000) {}

void TimeManager::begin() {
    timeClient.begin();
}

void TimeManager::update() {
    timeClient.update();
    formattedTime = timeClient.getFormattedTime();
}

String TimeManager::getFormattedTime() {
    return formattedTime;
}
