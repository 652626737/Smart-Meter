// EEPROMManager.h

#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include <Arduino.h>
#include <EEPROM.h>

// 定义一个结构体来保存传感器数据
struct SensorData {
    float voltage;
    float current;
    float power;
    float energy; 
    float frequency; 
    float pf;
};

class EEPROMManager {
public:
    EEPROMManager(int address);
    void writeSensorData(const SensorData& data);
    SensorData readSensorData();
    void clearData();
    void writeIntData(int data);
    int readIntData();
    void writeFloatData(float data);
    float readFloatData();

private:

    int address_;

};

#endif
