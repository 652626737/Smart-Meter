#define BLINKER_WIFI // 设置连接模式为 BLINKER_WIFI
#include <Blinker.h>
#include <PZEM004Tv30.h>
#include <Tasker.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <esp_ota_ops.h>
#include <NTPClient.h> // 包含NTPClient库
#include <WiFiUdp.h>   // 包含WiFiUDP库

#include <Config.h>                  // 包含 ESP32 的 WiFi 库
#include "ESP_Mail_Client_Wrapper.h" // 包含 ESP Mail 客户端库
#include "EEPROM_Manager.h"           // 包含 EEPROMManager 库
#include "NTPTime_Manager.h"          // 包含 NTPTimeManager 库
#include "Button.h"                  // 包含 Button 库

BlinkerNumber VOLTAGE("Voltage");
BlinkerNumber CURRENT("current");
BlinkerNumber POWER("power");
BlinkerText ENERGY("energy");
BlinkerNumber FREQUENCY("frequency");
BlinkerNumber PF("pf");

ESP_Mail_Client_Wrapper mailWrapper;
WiFiUDP ntpUDP;
NTPTimeManager ntpTimeManager(ntpServer, timeOffset, updateInterval); // NTP服务器, 时区偏移（以秒为单位）, 更新间隔（以毫秒为单位）

bool emailSent = false;            // 邮件是否已经发送
bool Debug = false;                // 调试模式
int Runday = 20;                   // 每月发送邮件日期
int updatesensorData_interval = 5; // 读取传感器数据的间隔时间（秒）
int updateEEPROM_interval = 60;    // 更新 EEPROM 的间隔时间（分钟）
int eepromAddress = 0;             // EEPROM 地址
unsigned long lastEmailTime = 0;
const unsigned long emailInterval = 1000; // 1 second interval

#define SENDMAIL_BUTTON_PIN 12
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#define PZEM_SERIAL Serial2

PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);

// 创建一个定时器对象
Tasker tasker;
Tasker updateEEPROMTask; // 创建一个定时器对象
Button sendmail_button(SENDMAIL_BUTTON_PIN);
SensorData sensorData;

// 读取传感器数据
SensorData readSensorData()
{
    SensorData localSensorData;
    if (Debug)
    {
        // 输入测试传感器数据...
        localSensorData.voltage = random(200, 250) / 10.0;  // 产生 20.0 到 25.0 之间的随机数
        localSensorData.current = random(50, 150) / 10.0;   // 产生 5.0 到 15.0 之间的随机数
        localSensorData.power = random(500, 1000);          // 产生 500 到 1000 之间的随机数
        localSensorData.energy = random(100, 1000) / 100.0; // 产生 1.0 到 10.0 之间的随机数
        localSensorData.frequency = random(50, 60);         // 产生 50 到 60 之间的随机数
        localSensorData.pf = random(80, 100) / 100.0;       // 产生 0.8 到 1.0 之间的随机数
    }
    else
    {
        // 读取传感器数据...
        localSensorData.voltage = pzem.voltage();
        localSensorData.current = pzem.current();
        localSensorData.power = pzem.power();
        localSensorData.energy = pzem.energy();
        localSensorData.frequency = pzem.frequency();
        localSensorData.pf = pzem.pf();
    }

    if (Debug)
    {

        Serial.println("-----------------test_data:----------------");
        Serial.print("Time: ");
        Serial.println(ntpTimeManager.getTime());
        Serial.print("test_Voltage: ");
        Serial.print(localSensorData.voltage);
        Serial.println("V");
        Serial.print("test_Current: ");
        Serial.print(localSensorData.current);
        Serial.println("A");
        Serial.print("test_Power: ");
        Serial.print(localSensorData.power);
        Serial.println("W");
        Serial.print("test_Energy: ");
        Serial.print(localSensorData.energy, 3);
        Serial.println("kWh");
        Serial.print("test_Frequency: ");
        Serial.print(localSensorData.frequency, 1);
        Serial.println("Hz");
        Serial.print("test_PF: ");
        Serial.println(localSensorData.pf);
    }
    return localSensorData;
}

void heartbeat()
{

    if (isnan(sensorData.voltage) || isnan(sensorData.current) || isnan(sensorData.power) || isnan(sensorData.energy) || isnan(sensorData.frequency) || isnan(sensorData.pf)) // 判断是否为NAN
    {
        sensorData.voltage = 0.0;
        sensorData.current = 0.0;
        sensorData.power = 0.0;
        sensorData.energy = 0.0;
        sensorData.frequency = 0.0;
        sensorData.pf = 0.0;
        // 发送实时数据到云平台
        Blinker.sendRtData("energy", sensorData.energy);
        Blinker.sendRtData("Voltage", sensorData.voltage);
        Blinker.sendRtData("current", sensorData.current);
        Blinker.sendRtData("power", sensorData.power);
        Blinker.sendRtData("frequency", sensorData.frequency);
        // Blinker.sendRtData("pf", pzem.pf());
    }
    else
    {
        Blinker.sendRtData("energy", sensorData.energy);
        Blinker.sendRtData("Voltage", sensorData.voltage);
        Blinker.sendRtData("current", sensorData.current);
        Blinker.sendRtData("power", sensorData.power);
        Blinker.sendRtData("frequency", sensorData.frequency);
        // Blinker.sendRtData("pf", pzem.pf());
    }
}

int calculateEEPROMAddressOffset()
{
    int eepromAddress = 0;

    // 计算每个字段的大小
    int sizeOfVoltage = sizeof(float);
    int sizeOfCurrent = sizeof(float);
    int sizeOfPower = sizeof(float);
    int sizeOfEnergy = sizeof(float);
    int sizeOfFrequency = sizeof(float);
    int sizeOfPF = sizeof(float);

    // 更新EEPROM地址偏移量
    eepromAddress += sizeOfVoltage;
    eepromAddress += sizeOfCurrent;
    eepromAddress += sizeOfPower;
    eepromAddress += sizeOfEnergy;
    eepromAddress += sizeOfFrequency;
    eepromAddress += sizeOfPF;
    // 打印EEPROM地址偏移量
    if (Debug)
    {
        Serial.print("EEPROM Address Offset: ");
        Serial.println(eepromAddress);
    }
    return eepromAddress;
}

/**
 * @brief 读取传感器数据并存储到EEPROM
 *
 * 从传感器中读取数据，并将其存储到EEPROM中。
 */
void readSensorDataAndStoreToEEPROM()
{
    EEPROMManager eepromManager(eepromAddress);
    eepromManager.writeSensorData(readSensorData());
    if (Debug)
    {
        SensorData TempData;
        // 获取EPPROM数据
        TempData = eepromManager.readSensorData();

        if (Debug)
        {
            // 打印读取到的传感器数据
            Serial.println("-----------------Read sensor data from EEPROM:----------------");
            Serial.print("Time: ");
            Serial.println(ntpTimeManager.getTime());
            Serial.print("Voltage: ");
            Serial.println(TempData.voltage);
            Serial.print("Current: ");
            Serial.println(TempData.current);
            Serial.print("Power: ");
            Serial.println(TempData.power);
            Serial.print("Energy: ");
            Serial.println(TempData.energy);
            Serial.print("Frequency: ");
            Serial.println(TempData.frequency);
            Serial.print("Power Factor: ");
            Serial.println(TempData.pf);
        }
        else
        {
            Serial.println("Failed to read sensor data from SPIFFS");
        };
    }
}

void updatesensorData()
{

    sensorData = readSensorData();
    if (Debug)
    {
        // 打印读取到的传感器数据
        // Serial.println("-----------------Read UPsensorData:----------------");
        // Serial.print("Voltage: ");
        // Serial.println(sensorData.voltage);
        // Serial.print("Current: ");
        // Serial.println(sensorData.current);
        // Serial.print("Power: ");
        // Serial.println(sensorData.power);
        // Serial.print("Energy: ");
        // Serial.println(sensorData.energy);
        // Serial.print("Frequency: ");
        // Serial.println(sensorData.frequency);
        // Serial.print("Power Factor: ");
        // Serial.println(sensorData.pf);
    }
}

void updateEEPROM()
{
    // 在这里执行需要定时执行的任务
    Serial.println("Timer task executed.");
    readSensorDataAndStoreToEEPROM();
}

/**
 * @brief 发送电费统计邮件
 *
 * 该函数用于发送电费统计邮件。首先获取当前年份和月份，然后创建EEPROM管理器对象。
 * 接着，读取上月传感器数据并进行有效性检查。如果上月传感器数据无效，则计算当前传感器数据的能量差值和电费，
 * 并发送电费统计邮件。如果上月传感器数据有效，则同样计算能量差值和电费，并发送电费统计邮件。
 * 最后，将当前传感器数据写入EEPROM。
 */
void sendEmail()
{
    // 获取当前年份和月份
    int month = ntpTimeManager.getMonth();
    int year = ntpTimeManager.getYear();

    // 计算EEPROM地址偏移量，只计算一次
    int eepromAddressOffset = calculateEEPROMAddressOffset();

    // 创建EEPROM管理器对象，只创建一次
    EEPROMManager eepromManager(eepromAddressOffset);

    // 读取上月传感器数据
    float lastSensorData;
    lastSensorData = eepromManager.readFloatData();

    float energyDifference = 0.0;
    float electricityCost = 0.0;
    String emailContent = "";

    // 检查上月传感器数据的有效性
    if (lastSensorData == 0.00 || isnan(lastSensorData) || lastSensorData < 0.000)
    {
        // 上月起始能量使用0.000作为默认值
        emailContent += "上月起始能量: " + String(0.000) + "\r\n\r\n";
    }
    else
    {
        emailContent += "上月起始能量:" + String(lastSensorData) + "\r\n\r\n";
    }

    // 计算能量差值和电费（现在只需做一次）
    if (sensorData.voltage != 0.00 && sensorData.current != 0.00 && sensorData.power != 0.00)
    {
        energyDifference = sensorData.energy - lastSensorData;
        if (energyDifference > 0.000)
        {
            electricityCost = energyDifference * 1.5; // 假设电费等于能量差值
            emailContent += "本月终止能量:" + String(sensorData.energy, 3) + "\r\n\r\n";
            emailContent += "本月能量差值:" + String(energyDifference, 3) + "\r\n\r\n";
            emailContent += "本月电费:" + String(electricityCost, 3) + "\r\n\r\n";
        }
        else
        {
            Serial.println("能量差值小于或等于0，无法计算电费。");
            if (Debug)
            {
                eepromManager.writeFloatData(1.00);
            }
            return;
        }
    }
    else
    {
        Serial.println("无效的传感器数据。");
        return;
    }

    // 发送电费统计邮件
    String subject = String(year) + "年-" + String(month) + "月电费统计";
    mailWrapper.send_mail(subject, emailContent);

    // 将当前传感器数据写入EEPROM
    eepromManager.writeFloatData(sensorData.energy);
}

void printESP32Info()
{
    // 获取ESP32-SOLO的芯片信息
    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);

    // 获取Flash存储器信息
    const esp_partition_t *partition = esp_ota_get_running_partition();
    const esp_partition_t *flashPartition = esp_ota_get_next_update_partition(partition);
    uint32_t flashSize = spi_flash_get_chip_size(); // 获取Flash存储器的大小（字节）

    // 获取ESP32-SOLO的唯一ID
    uint64_t chipId = ESP.getEfuseMac(); // 获取芯片的唯一ID

    // 获取内存使用情况
    uint32_t totalHeap = ESP.getHeapSize(); // 获取总堆内存大小（字节）
    uint32_t freeHeap = ESP.getFreeHeap();  // 获取剩余堆内存大小（字节）

    // 打印ESP32-SOLO信息
    Serial.println("ESP32-SOLO Information:");
    Serial.print("Chip Model: ");
    Serial.println(chipInfo.model);
    Serial.print("Chip Revision: ");
    Serial.println(chipInfo.revision);
    Serial.print("Chip Cores: ");
    Serial.println(chipInfo.cores);
    Serial.print("CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");
    Serial.print("Flash Size: ");
    Serial.print(flashSize / (1024 * 1024)); // 将字节转换为兆字节
    Serial.println(" MB");
    Serial.print("Flash Partition: ");
    Serial.print(flashPartition->label);
    Serial.print(" (Size: ");
    Serial.print(flashPartition->size / (1024));
    Serial.println(" KB)");
    Serial.print("Chip Unique ID: 0x");
    Serial.println(chipId, HEX);
    Serial.print("Total Heap Size: ");
    Serial.print(totalHeap);
    Serial.println(" bytes");
    Serial.print("Free Heap Size: ");
    Serial.print(freeHeap);
    Serial.println(" bytes");
}

void sendMonthlyEmail()
{
    // 获取当前日期和时间
    int day = ntpTimeManager.getDay();
    int month = ntpTimeManager.getMonth();
    int year = ntpTimeManager.getYear();
    int hour = ntpTimeManager.getHour();

    // 检查日期、时间和是否已发送邮件的标志
    if (day == Runday && !emailSent && hour == 10)
    {
        sendEmail();
        emailSent = true; // 邮件发送成功后，将 emailSent 标志设置为 true
    }
    else if (day != Runday)
    {
        emailSent = false; // 如果不是发送邮件的日子，则重置标志
    }
}

/**
 * @brief 按下按钮时发送邮件
 *
 * 当 sendmail_button 按钮被按下时，调用 sendEmail() 函数发送邮件。
 */
void buttonPressedSendEmail()
{
    if (sendmail_button.isPressed())
    {
        unsigned long currentTime = millis();
        if (currentTime - lastEmailTime >= emailInterval)
        {
            sendEmail();
            lastEmailTime = currentTime;
        }
    }
}

void setup()
{
    // 初始化串口通信，波特率为115200
    Serial.begin(115200);
    // 等待1秒
    delay(1000);

    // 输出欢迎信息
    Serial.print("Welcome to ESP32 Smart Meters");
    // 打印ESP32相关信息
    printESP32Info();

    // 初始化Blinker，并设置认证信息、SSID和密码
    Blinker.begin(auth, ssid, password);
    // 注册心跳包
    Blinker.attachHeartbeat(heartbeat); // 注册心跳包
    // 设置定时器，执行一次UPsensorData函数更新
    tasker.setInterval(updatesensorData, updatesensorData_interval * 1000, 0);
    // 设置定时器，按照设定的时间间隔执行updateEEPROMTask函数
    updateEEPROMTask.setInterval(updateEEPROM, updateEEPROM_interval * 1000 * 60, 0);
    // 设置定时器，每分钟执行一次sendEmail函数

    // 连接Wi-Fi
    WiFi.begin(ssid, password);
    // 输出正在连接到Wi-Fi的信息
    Serial.print("连接到 Wi-Fi");
    // 循环等待Wi-Fi连接成功
    while (WiFi.status() != WL_CONNECTED)
    {
        // 输出等待提示
        Serial.print(".");
        // 等待300毫秒
        delay(300);
    }
    // 换行
    Serial.println();
    // 输出已连接的IP地址
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    // 换行
    Serial.println();

    // 初始化NTP时间管理器
    ntpTimeManager.begin();

    // 初始化EEPROM管理器
    EEPROMManager eepromManager_1(eepromAddress);
    // 定义并初始化SensorData结构体变量readData
    SensorData readData;
    // 从EEPROM中读取传感器数据并保存到readData中
    readData = eepromManager_1.readSensorData();

    // 输出从EEPROM中读取的传感器数据
    Serial.println("Sensor data read from EEPROM:");
    Serial.print("Voltage: ");
    Serial.println(readData.voltage);
    Serial.print("Current: ");
    Serial.println(readData.current);
    Serial.print("Power: ");
    Serial.println(readData.power);
    Serial.print("Energy: ");
    Serial.println(readData.energy);
    Serial.print("Frequency: ");
    Serial.println(readData.frequency);
    Serial.print("PF: ");
    Serial.println(readData.pf);
}

void loop()
{
    // 更新网络时间
    ntpTimeManager.updateTime();

    // 运行 Blinker 的任务
    Blinker.run();

    // 在 loop() 中调用 timer.run()，用于执行定时器任务
    // 在 loop() 中调用 tasker.loop()，用于执行定时任务
    tasker.loop();

    // 运行 EEPROM 定时器任务
    updateEEPROMTask.loop();

    // 发送月度邮件
    sendMonthlyEmail();
    buttonPressedSendEmail();
    // loop() 中的其他任务可以继续在这里添加
    // Other tasks in loop can continue here
}