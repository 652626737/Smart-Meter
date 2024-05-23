#include "LittleFS_Manager.h"

LittleFSManager::LittleFSManager() {}

bool LittleFSManager::init() {
    return LittleFS.begin();
}

/**
 * @brief 写入传感器数据到文件
 *
 * 将给定的传感器数据写入到指定的文件中。
 *
 * @param data 传感器数据
 *
 * @return 写入是否成功，成功返回 true，否则返回 false
 */
bool LittleFSManager::writeSensorData(const SensorData& data) {
    File file = LittleFS.open(filePath, "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }

    size_t bytesWritten = file.write((uint8_t*)&data, sizeof(SensorData));
    file.close();

    return bytesWritten == sizeof(SensorData);
}

/**
 * @brief 读取传感器数据
 *
 * 从LittleFS文件系统中读取传感器数据，并返回读取到的数据。
 *
 * @return 读取到的传感器数据
 */
SensorData LittleFSManager::readSensorData() {
    SensorData data;

    File file = LittleFS.open(filePath, "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return data;
    }

    file.read((uint8_t*)&data, sizeof(SensorData));
    file.close();

    return data;
}

/**
 * @brief 清除数据
 *
 * 从 LittleFS 文件系统中删除指定的文件。
 */
void LittleFSManager::clearData() {
    LittleFS.remove(filePath);
}
