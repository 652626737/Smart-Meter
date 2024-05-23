#include "NTPTime_Manager.h"

NTPTimeManager::NTPTimeManager(const char* ntpServer, long timeOffset, unsigned long updateInterval)
    : timeClient_(ntpUDP_, ntpServer, timeOffset, updateInterval) {}

void NTPTimeManager::begin() {
    timeClient_.begin();
}

void NTPTimeManager::updateTime() {
    timeClient_.update();
}

String NTPTimeManager::getFormattedDate() {
    unsigned long epochTime = timeClient_.getEpochTime();
    tm* timeInfo = localtime((time_t*)&epochTime);

    char buffer[11];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday);
    return String(buffer);
}

String NTPTimeManager::getWeekDay() {
    unsigned long epochTime = timeClient_.getEpochTime();
    tm* timeInfo = localtime((time_t*)&epochTime);

    const char* weekDays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return String(weekDays[timeInfo->tm_wday]);
}

String NTPTimeManager::getTime() {
    unsigned long epochTime = timeClient_.getEpochTime();
    tm* timeInfo = localtime((time_t*)&epochTime);

    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
    return String(buffer);
}


int NTPTimeManager::getYear() {
    unsigned long epochTime = timeClient_.getEpochTime();
    tm* timeInfo = localtime((time_t*)&epochTime);
    return timeInfo->tm_year + 1900;
}

int NTPTimeManager::getMonth() {
    unsigned long epochTime = timeClient_.getEpochTime();
    tm* timeInfo = localtime((time_t*)&epochTime);
    return timeInfo->tm_mon + 1;
}

int NTPTimeManager::getDay() {
    unsigned long epochTime = timeClient_.getEpochTime();
    tm* timeInfo = localtime((time_t*)&epochTime);
    return timeInfo->tm_mday;
}

int NTPTimeManager::getHour() {
    unsigned long epochTime = timeClient_.getEpochTime();
    tm* timeInfo = localtime((time_t*)&epochTime);
    return timeInfo->tm_hour;
}

int NTPTimeManager::getMinute() {
    unsigned long epochTime = timeClient_.getEpochTime();
    tm* timeInfo = localtime((time_t*)&epochTime);
    return timeInfo->tm_min;
}

int NTPTimeManager::getSecond() {
    unsigned long epochTime = timeClient_.getEpochTime();
    tm* timeInfo = localtime((time_t*)&epochTime);
    return timeInfo->tm_sec;
}