#include "Spiffs_Manager.h"

SpiffsManager::SpiffsManager() : initialized(false) {}

bool SpiffsManager::begin() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return false;
    }
    initialized = true;
    return true;
}

bool SpiffsManager::writeSensorData(const char* path, const SensorData& data) {
    if (!initialized) return false;
    
    File file = SPIFFS.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    file.write((const uint8_t*)&data, sizeof(SensorData));
    file.close();
    return true;
}

bool SpiffsManager::readSensorData(const char* path, SensorData& data) {
    if (!initialized) return false;
    
    File file = SPIFFS.open(path, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    }
    file.read((uint8_t*)&data, sizeof(SensorData));
    file.close();
    return true;
}

bool SpiffsManager::clearData(const char* path) {
    if (!initialized) return false;
    
    if (!SPIFFS.remove(path)) {
        Serial.println("Failed to remove file");
        return false;
    }
    return true;
}
