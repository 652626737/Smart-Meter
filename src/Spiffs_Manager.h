#ifndef SPIFFS_MANAGER_H
#define SPIFFS_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

// 定义一个结构体来保存传感器数据
struct SensorData {
    float voltage;
    float current;
    float power;
    float energy;
    float frequency;
    float pf;
};

class SpiffsManager {
public:
    SpiffsManager();
    bool begin();
    bool writeSensorData(const char* path, const SensorData& data);
    bool readSensorData(const char* path, SensorData& data);
    bool clearData(const char* path);

private:
    bool initialized;
};

#endif
