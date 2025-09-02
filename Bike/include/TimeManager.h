#pragma once
#include <RtcDS1302.h>
#include <ThreeWire.h>
#include <WiFiUdp.h>
#include <NTP.h>

class TimeManager {
public:
    TimeManager();
    void begin();
    void syncWithNTP();
    String nowString();
private:
    ThreeWire wire;
    RtcDS1302<ThreeWire> rtc;
    WiFiUDP udp;
    NTP ntp;
};
