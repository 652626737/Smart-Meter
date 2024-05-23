#ifndef LITTLEFS_MANAGER_H
#define LITTLEFS_MANAGER_H

#include <Arduino.h>
#include <LittleFS.h>

// 定义一个结构体来保存传感器数据
struct SensorData {
    float voltage;
    float current;
    float power;
    float energy; 
    float frequency; 
    float pf;
};

class LittleFSManager {
public:
    LittleFSManager();

    bool init();
    bool writeSensorData(const SensorData& data);
    SensorData readSensorData();
    void clearData();

private:
    const char* filePath = "/sensor_data.txt";
    const size_t maxFileSize = sizeof(SensorData);
};

#endif
