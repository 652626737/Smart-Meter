#ifndef NTP_TIME_MANAGER_H
#define NTP_TIME_MANAGER_H

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

class NTPTimeManager {
public:
    NTPTimeManager(const char* ntpServer, long timeOffset, unsigned long updateInterval);
    void begin();
    void updateTime();
    String getFormattedDate();
    String getWeekDay();
    String getTime();
    int getYear();
    int getMonth();
    int getDay();
    int getHour();
    int getMinute();
    int getSecond();

private:
    WiFiUDP ntpUDP_;
    NTPClient timeClient_;
};

#endif
