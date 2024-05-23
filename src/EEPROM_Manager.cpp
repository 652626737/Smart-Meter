// EEPROMManager.cpp

#include "EEPROM_Manager.h"

EEPROMManager::EEPROMManager(int address) : address_(address)
{
    EEPROM.begin(512);
}

/**
 * @brief 将传感器数据写入EEPROM中。
 * 该函数负责将传入的传感器数据写入EEPROM中，并确保数据正确写入。
 * 如果第一次写入失败，将尝试多次写入。
 * 
 * @param data 要写入的传感器数据，类型为SensorData结构体。
 */
void EEPROMManager::writeSensorData(const SensorData &data)
{
    const int maxAttempts = 5; // 设定最大尝试次数
    int attempt = 0;       // 将 attempt 声明移到循环外部
    for (; attempt < maxAttempts; attempt++)
    {
        byte *ptr = (byte *)&data;
        for (int i = 0; i < sizeof(SensorData); i++)
        {
            EEPROM.write(address_ + i, *ptr);
            ptr++;
        }
        if (EEPROM.commit()) // 如果提交成功，则退出循环
        {
            break;
        }
        Serial.println("EEPROM commit failed, retrying...");
        delay(100); // 等待一段时间后再重试
    }
    if (attempt == maxAttempts - 1) // 如果达到最大尝试次数仍失败
    {
        Serial.println("EEPROM commit failed after multiple attempts!");
    }
}

/**
 * @brief 向 EEPROM 中写入整型数据
 *
 * 该函数负责将指定的整型数据写入 EEPROM 中预设的地址，并尝试提交更改。如果首次写入和提交失败，
 * 函数将等待一段时间后进行重试，此过程将重复最多 maxAttempts 次。
 * 如果在最大尝试次数内操作成功，则结束循环。如果达到最大尝试次数后仍无法成功写入和提交，
 * 则函数会打印错误信息。
 *
 * @param value 要写入 EEPROM 的整型数据。
 */
void EEPROMManager::writeIntData(int value)
{
    const int maxAttempts = 5; // 定义尝试写入的最大次数
    int attempt = 0;           // 初始化 attempt 变量
    for (; attempt < maxAttempts; attempt++) // 注意这里不再声明 attempt
    {
        EEPROM.write(address_, value); // 尝试写入数据
        if (EEPROM.commit()) // 尝试提交，并检查是否成功
        {
            break; // 如果提交成功，则跳出循环
        }
        Serial.println("EEPROM commit failed, retrying..."); // 写入或提交失败时，打印重试信息
        delay(100); // 等待一段时间后进行重试
    }
    if (attempt == maxAttempts) // 如果达到最大尝试次数仍未成功
    {
        Serial.println("EEPROM commit failed after multiple attempts!"); // 打印错误信息
    }
}


/**
 * @brief 读取 EEPROM 中的整型数据
 *
 * 从 EEPROM 中的指定地址读取一个整型数据，并返回该数据。
 *
 * @param address EEPROM 地址
 *
 * @return EEPROM 中指定地址的整型数据
 */
int EEPROMManager::readIntData()
{
    int value = EEPROM.readInt(address_);
    return value;
}


/**
 * @brief 写入浮点数数据到EEPROM中
 *
 * 将给定的浮点数数据写入EEPROM中，并进行多次尝试，以确保数据成功写入。
 *
 * @param value 要写入的浮点数数据
 */
void EEPROMManager::writeFloatData(float value)
{
    const int maxAttempts = 5; // 定义尝试写入的最大次数
    int attempt = 0;           // 初始化 attempt 变量
    byte* p = (byte*)(void*)&value; // 将浮点数转换为字节数据块的指针
    for (; attempt < maxAttempts; attempt++) // 注意这里不再声明 attempt
    {
        for (int i = 0; i < sizeof(float); i++) {
            EEPROM.write(address_ + i, *p++); // 将字节数据块写入EEPROM
        }
        if (EEPROM.commit()) // 尝试提交，并检查是否成功
        {
            break; // 如果提交成功，则跳出循环
        }
        Serial.println("EEPROM commit failed, retrying..."); // 写入或提交失败时，打印重试信息
        delay(100); // 等待一段时间后进行重试
    }
    if (attempt == maxAttempts) // 如果达到最大尝试次数仍未成功
    {
        Serial.println("EEPROM commit failed after multiple attempts!"); // 打印错误信息
    }
}


/**
 * @brief 读取浮点型数据
 *
 * 从EEPROM中读取浮点型数据并返回。
 *
 * @return 读取到的浮点数值
 */
float EEPROMManager::readFloatData()
{
    float value; // 用于存储读取的浮点数值

    byte* p = (byte*)(void*)&value; // 将浮点数的内存地址转换为字节数据块的指针

    for (int i = 0; i < sizeof(float); i++) {
        *p++ = EEPROM.read(address_ + i); // 逐个字节地读取EEPROM中的数据，并存储到浮点数的内存中
    }

    return value; // 返回读取到的浮点数值
}


SensorData EEPROMManager::readSensorData()
{
    SensorData data;
    byte *ptr = (byte *)&data;

    for (int i = 0; i < sizeof(SensorData); i++)
    {
        *ptr = EEPROM.read(address_ + i);
        ptr++;
    }
    return data;
}

void EEPROMManager::clearData()
{
    SensorData defaultData;
    EEPROM.put(address_, defaultData);
    EEPROM.commit();
}
