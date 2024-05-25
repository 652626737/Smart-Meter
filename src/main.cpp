#define BLINKER_WIFI // 设置连接模式为 BLINKER_WIFI
#include <Blinker.h>
#include <PZEM004Tv30.h>
#include <Tasker.h>
#include <EEPROM.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <esp_ota_ops.h>
#include <NTPClient.h> // 包含NTPClient库
#include <WiFiUdp.h>   // 包含WiFiUDP库
#include <cJSON.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Arduino.h>

// #include <Config.h>                  // 包含 ESP32 的 WiFi 库
#include "ESP_Mail_Client_Wrapper.h" // 包含 ESP Mail 客户端库
#include "EEPROM_Manager.h"          // 包含 EEPROMManager 库
#include "NTPTime_Manager.h"         // 包含 NTPTimeManager 库
#include "Button.h"

#define FILESYSTEM LittleFS
// You only need to format the filesystem once
#define FORMAT_FILESYSTEM false
#define DBG_OUTPUT_PORT Serial

#define APP_VERSION "0.1.1"

String ssid = "";
String password = "";
String smtpServer = "";
int smtpPort = 465;
String emailFrom = "";
String passwordemail = "";
String emailTo = "";
String auth = "";
int sendday = 1;
int sendtime = 10;

String apSSID;

File fsUploadFile;

/************************************************************************************/

BlinkerNumber VOLTAGE("Voltage");
BlinkerNumber CURRENT("current");
BlinkerNumber POWER("power");
BlinkerText ENERGY("energy");
BlinkerNumber FREQUENCY("frequency");
BlinkerNumber PF("pf");

const char *host = "esp32-solo1";

ESP_Mail_Client_Wrapper mailWrapper(
    smtpServer, smtpPort, emailFrom, passwordemail, emailTo);
WiFiUDP ntpUDP;

extern WebServer server(80);

const long updateInterval = 60000; // 更新间隔，以毫秒为单位
const char *ntpServer = smtpServer.c_str();
const long timeOffset = 0; // 时区偏移，以秒为单位

NTPTimeManager ntpTimeManager(ntpServer, timeOffset, updateInterval); // NTP服务器, 时区偏移（以秒为单位）, 更新间隔（以毫秒为单位）

bool emailSent = false;            // 邮件是否已经发送
bool Debug = false;                // 调试模式
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

// holds the current upload
// File fsUploadFile;

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
void sendEmail(bool isUpdate = true)
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
    if (isUpdate)
    {
        eepromManager.writeFloatData(sensorData.energy);
    }
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
    if (day == sendday && !emailSent && hour == sendtime)
    {
        sendEmail();
        emailSent = true; // 邮件发送成功后，将 emailSent 标志设置为 true
    }
    else if (day != sendday)
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
            sendEmail(false);
            lastEmailTime = currentTime;
        }
    }
}

void wifiSetup()
{
    DBG_OUTPUT_PORT.println("wifiSetup");
    uint8_t mac[6] = {0};
    char ssidStr[32] = {0};
    int attempts = 0;

    WiFi.disconnect(true);
    delay(1000);

    Serial.println("WiFi: Set mode to WIFI_AP_STA");
    WiFi.mode(WIFI_AP_STA);

    // Retry connecting to Wi-Fi network up to 5 times
    while (attempts < 5 && WiFi.status() != WL_CONNECTED)
    {
        Serial.printf("Attempt %d to connect to Wi-Fi...\n", attempts + 1);

        // Try to connect to the Wi-Fi network
        if (WiFi.begin(ssid.c_str(), password.c_str()) == WL_CONNECTED)
        {
            Serial.println("Connected to Wi-Fi.");
            break;
        }

        attempts++;
        delay(1000); // Wait 1 seconds before retrying
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Failed to connect to Wi-Fi after 5 attempts. Starting Access Point mode.");

        WiFi.mode(WIFI_AP);
        // Generate SSID for Access Point
        WiFi.softAPmacAddress(mac);
        sprintf(ssidStr, "ESP-%02X%02X%02X", mac[3], mac[4], mac[5]);

        // Create soft Access Point
        if (WiFi.softAP(ssidStr))
        {
            apSSID = String(ssidStr);
            Serial.println("WiFi: softAP has been established");
            Serial.printf("WiFi: please connect to the %s\r\n", apSSID);
        }
        else
        {
            Serial.println("WiFi: failed to create softAP");
        }
    }
}

void appLoadDateBase(void)
{

    DBG_OUTPUT_PORT.println("appLoadDateBase");

    File dbfile = FILESYSTEM.open("/db.json", "r");
    if (!dbfile)
    {
        DBG_OUTPUT_PORT.println("Error opening file.");
        return;
    }

    char *buffer = (char *)malloc(dbfile.size());

    while (dbfile.available())
    {
        dbfile.readBytes(buffer, dbfile.size());
    }

    DBG_OUTPUT_PORT.println(buffer);

    cJSON *rootObject = cJSON_ParseWithLength(buffer, dbfile.size());
    if (rootObject == NULL)
    {
        dbfile.close();
        return;
    }

    cJSON *wifiObject = cJSON_GetObjectItem(rootObject, "wifi");

    cJSON *ssidObject = cJSON_GetObjectItem(wifiObject, "ssid");
    cJSON *psdObject = cJSON_GetObjectItem(wifiObject, "password");

    cJSON *emailObject = cJSON_GetObjectItem(rootObject, "email");
    cJSON *smserverObject = cJSON_GetObjectItem(emailObject, "smtpserver");
    cJSON *emailFromObject = cJSON_GetObjectItem(emailObject, "emailFrom");
    cJSON *pawoemaObject = cJSON_GetObjectItem(emailObject, "passwordemail");
    cJSON *emailToObject = cJSON_GetObjectItem(emailObject, "emailTo");

    cJSON *blinkerObject = cJSON_GetObjectItem(rootObject, "blinker");
    cJSON *authObject = cJSON_GetObjectItem(blinkerObject, "auth");

    cJSON *senderObject = cJSON_GetObjectItem(rootObject, "sender");
    cJSON *SdayObjectObject = cJSON_GetObjectItem(senderObject, "sendday");
    cJSON *SendTimeObjectObject = cJSON_GetObjectItem(senderObject, "sendtime");

    ssid = ssidObject->valuestring;
    password = psdObject->valuestring;
    smtpServer = smserverObject->valuestring;
    emailFrom = emailFromObject->valuestring;
    passwordemail = pawoemaObject->valuestring;
    emailTo = emailToObject->valuestring;
    auth = authObject->valuestring;
    sendday = SdayObjectObject->valueint;
    sendtime = SendTimeObjectObject->valueint;

    DBG_OUTPUT_PORT.print("ssid: ");
    DBG_OUTPUT_PORT.println(ssid);
    DBG_OUTPUT_PORT.print("password: ");
    DBG_OUTPUT_PORT.println(password);

    free(buffer);
    cJSON_Delete(rootObject);
    dbfile.close();

    DBG_OUTPUT_PORT.println("appLoadDateBase");
}

void getStatus()
{
    cJSON *rspObject = NULL;
    cJSON *sysObject = NULL;
    cJSON *archObject = NULL;
    cJSON *memObject = NULL;
    cJSON *fsObject = NULL;
    cJSON *apObject = NULL;
    cJSON *staObject = NULL;

    rspObject = cJSON_CreateObject();
    if (rspObject == NULL)
    {
        goto OUT1;
    }

    sysObject = cJSON_CreateObject();
    if (sysObject == NULL)
    {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "sys", sysObject);
    cJSON_AddStringToObject(sysObject, "model", "ESP32S3 Dev Module");
    cJSON_AddStringToObject(sysObject, "fw", APP_VERSION);
    cJSON_AddStringToObject(sysObject, "sdk", ESP.getSdkVersion());
    archObject = cJSON_CreateObject();
    if (archObject == NULL)
    {
        goto OUT;
    }
    cJSON_AddItemToObject(sysObject, "arch", archObject);
    cJSON_AddStringToObject(archObject, "mfr", "Espressif");
    cJSON_AddStringToObject(archObject, "model", ESP.getChipModel());
    cJSON_AddNumberToObject(archObject, "revision", ESP.getChipRevision());
    if (!strncmp(ESP.getChipModel(), "ESP32-S3", strlen("ESP32-S3")))
    {
        cJSON_AddStringToObject(archObject, "cpu", "XTensa® dual-core LX7");
    }
    else if (!strncmp(ESP.getChipModel(), "ESP32-S2", strlen("ESP32-S2")))
    {
        cJSON_AddStringToObject(archObject, "cpu", "XTensa® single-core LX7");
    }
    else if (!strncmp(ESP.getChipModel(), "ESP32-C3", strlen("ESP32-C3")))
    {
        cJSON_AddStringToObject(archObject, "cpu", "RISC-V");
    }
    else if (!strncmp(ESP.getChipModel(), "ESP32", strlen("ESP32")))
    {
        cJSON_AddStringToObject(archObject, "cpu", "XTensa® dual-core LX6");
    }
    cJSON_AddNumberToObject(archObject, "freq", ESP.getCpuFreqMHz());

    memObject = cJSON_CreateObject();
    if (memObject == NULL)
    {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "mem", memObject);
    cJSON_AddNumberToObject(memObject, "total", ESP.getHeapSize());
    cJSON_AddNumberToObject(memObject, "free", ESP.getFreeHeap());

    fsObject = cJSON_CreateObject();
    if (fsObject == NULL)
    {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "fs", fsObject);
    cJSON_AddNumberToObject(fsObject, "total", FILESYSTEM.totalBytes());
    cJSON_AddNumberToObject(fsObject, "used", FILESYSTEM.usedBytes());
    cJSON_AddNumberToObject(fsObject, "free", FILESYSTEM.totalBytes() - FILESYSTEM.usedBytes());

    apObject = cJSON_CreateObject();
    if (apObject == NULL)
    {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "ap", apObject);
    cJSON_AddStringToObject(apObject, "ssid", WiFi.softAPSSID().c_str());
    cJSON_AddNumberToObject(apObject, "num", WiFi.softAPgetStationNum());

    staObject = cJSON_CreateObject();
    if (staObject == NULL)
    {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "sta", staObject);
    cJSON_AddStringToObject(staObject, "ssid", ssid.c_str());
    cJSON_AddStringToObject(staObject, "status", WiFi.isConnected() ? "connected" : "disconnect");

    server.send(200, "application/json", cJSON_Print(rspObject));
OUT:
    cJSON_Delete(rspObject);
OUT1:
    return;
}

void getConfig()
{
    File file = FILESYSTEM.open("/db.json", "r");
    size_t sent = server.streamFile(file, "application/json");
    file.close();
    return;
}

void postConfig()
{

    cJSON *reqObject = NULL;
    cJSON *wifiObject = NULL;
    cJSON *emailObject = NULL;
    cJSON *blinkerObject = NULL;
    cJSON *SenderObject = NULL;

    bool flag = false;

    String content = server.arg("plain");

    reqObject = cJSON_Parse(content.c_str());
    if (reqObject == NULL)
    {
        Serial.println("JSON parse error");
        Serial.print("payload: ");
        Serial.println(server.arg("plain"));
        return;
    }

    wifiObject = cJSON_GetObjectItem(reqObject, "wifi");
    if (wifiObject)
    {
        cJSON *ssidObject = cJSON_GetObjectItem(wifiObject, "ssid");
        if (String(ssidObject->valuestring) != ssid)
        {
            ssid = ssidObject->valuestring;
            flag = true;
        }
        cJSON *psdObject = cJSON_GetObjectItem(wifiObject, "password");
        if (String(psdObject->valuestring) != password)
        {
            password = psdObject->valuestring;
            flag = true;
        }
    }

    emailObject = cJSON_GetObjectItem(reqObject, "email");
    if (emailObject)
    {
        cJSON *smserverObject = cJSON_GetObjectItem(emailObject, "smtpserver");
        if (String(smserverObject->valuestring) != smtpServer)
        {
            smtpServer = smserverObject->valuestring;
            flag = true;
        }
        cJSON *emailFromObject = cJSON_GetObjectItem(emailObject, "emailFrom");
        if (String(emailFromObject->valuestring) != emailFrom)
        {
            emailFrom = emailFromObject->valuestring;
            flag = true;
        }
        cJSON *pawoemaObject = cJSON_GetObjectItem(emailObject, "passwordemail");
        if (String(pawoemaObject->valuestring) != passwordemail)
        {
            passwordemail = pawoemaObject->valuestring;
            flag = true;
        }

        cJSON *emailToObject = cJSON_GetObjectItem(emailObject, "emailTo");
        if (String(emailToObject->valuestring) != emailTo)
        {
            emailTo = emailToObject->valuestring;
            flag = true;
        }
    }

    blinkerObject = cJSON_GetObjectItem(reqObject, "blinker");
    if (blinkerObject)
    {
        cJSON *authObject = cJSON_GetObjectItem(blinkerObject, "auth");
        if (String(authObject->valuestring) != auth)
        {
            auth = authObject->valuestring;
            flag = true;
        }
    }

    SenderObject = cJSON_GetObjectItem(reqObject, "sender");
    if (SenderObject)
    {
        cJSON *SdayObjectObject = cJSON_GetObjectItem(SenderObject, "sendday");
        if (SdayObjectObject->valueint != sendday)
        {
            sendday = SdayObjectObject->valueint;
            flag = true;
        }
        cJSON *SendTimeObjectObject = cJSON_GetObjectItem(SenderObject, "sendtime");
        if (SendTimeObjectObject->valueint != sendtime)
        {
            sendtime = SendTimeObjectObject->valueint;
            flag = true;
        }
    }

    File configfile = FILESYSTEM.open("/db.json", FILE_WRITE);
    configfile.write((const uint8_t *)content.c_str(), content.length());
    configfile.close();

    server.send(201, "application/json", server.arg("plain"));

    if (flag)
    {
        // WiFi.disconnect();
        // delay(1000);
        // WiFi.begin(ssid.c_str(), password.c_str());
        ESP.restart();
    }

    return;
}

void onServeStaticSubDir(File &dir, String topDir)
{
    File file = dir.openNextFile();
    while (file)
    {
        String path = String(file.path());
        String uri = path.substring(path.indexOf(topDir) + topDir.length(), path.indexOf(".gz"));
        server.serveStatic(uri.c_str(), FILESYSTEM, path.c_str());
        DBG_OUTPUT_PORT.print("uri: ");
        DBG_OUTPUT_PORT.println(uri);
        DBG_OUTPUT_PORT.print("path: ");
        DBG_OUTPUT_PORT.println(path);
        file = dir.openNextFile();
    }
}

void onServeStatic(String dir)
{
    File wwwDir = FILESYSTEM.open(dir);

    if (wwwDir.isDirectory())
    {
        File file = wwwDir.openNextFile();
        while (file)
        {
            if (file.isDirectory())
            {
                file = FILESYSTEM.open(file.path());
                onServeStaticSubDir(file, dir);
            }
            else
            {
                String path = String(file.path());
                String uri = path.substring(path.indexOf(dir) + dir.length(), path.indexOf(".gz"));
                server.serveStatic(uri.c_str(), FILESYSTEM, path.c_str());
                DBG_OUTPUT_PORT.print("uri: ");
                DBG_OUTPUT_PORT.println(uri);
                DBG_OUTPUT_PORT.print("path: ");
                DBG_OUTPUT_PORT.println(path);
            }
            file = wwwDir.openNextFile();
        }
    }
}

String formatBytes(size_t bytes)
{
    if (bytes < 1024)
    {
        return String(bytes) + "B";
    }
    else if (bytes < (1024 * 1024))
    {
        return String(bytes / 1024.0) + "KB";
    }
    else if (bytes < (1024 * 1024 * 1024))
    {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    }
    else
    {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
    }
}

void setup()
{
    // 初始化串口通信，波特率为115200
    Serial.begin(115200);
    // 等待1秒
    delay(1000);
    Serial.println("_________________________________________________________________________________________");
    if (FORMAT_FILESYSTEM)
        FILESYSTEM.format();
    FILESYSTEM.begin();
    {
        File root = FILESYSTEM.open("/");
        File file = root.openNextFile();
        while (file)
        {
            String fileName = file.name();
            size_t fileSize = file.size();
            DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
            file = root.openNextFile();
        }
        DBG_OUTPUT_PORT.printf("\n");
    }
    appLoadDateBase();

    wifiSetup();

    server.serveStatic("/", FILESYSTEM, "/www/index.html");
    onServeStatic("/www");

    server.on("/api/v1/status", HTTP_GET, getStatus);
    server.on("/api/v1/config", HTTP_GET, getConfig);
    server.on("/api/v1/config", HTTP_POST, postConfig);

    server.on(
        "/api/v1/update", HTTP_POST, []()
        {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart(); },
        []()
        {
            HTTPUpload &upload = server.upload();
            if (upload.status == UPLOAD_FILE_START)
            {
                Serial.setDebugOutput(true);
                Serial.printf("Update: %s\n", upload.filename.c_str());
                if (!Update.begin())
                { // start with max available size
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE)
            {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_END)
            {
                if (Update.end(true))
                { // true to set the size to the current progress
                    Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                }
                else
                {
                    Update.printError(Serial);
                }
                Serial.setDebugOutput(false);
            }
            else
            {
                Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
            }
        });
    server.onNotFound([]()
                      { server.send(404, "text/plain", "FileNotFound"); });

    server.begin();
    DBG_OUTPUT_PORT.println("HTTP server started");

    Serial.println("_________________________________________________________________________________________");
    // 输出欢迎信息
    Serial.println("_______________Welcome to ESP32 Smart Meters_______________");
    // 打印ESP32相关信息
    printESP32Info();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi connected, initializing Blinker.");

        const char *Blinker_auth = auth.c_str();
        const char *Blinker_ssid = ssid.c_str();
        const char *Blinker_password = password.c_str();
        // 初始化Blinker，并设置认证信息、SSID和密码
        Blinker.begin(Blinker_auth, Blinker_ssid, Blinker_password);
        Blinker.attachHeartbeat(heartbeat); // 注册心跳包

        // 设置定时器，执行一次UPsensorData函数更新
        tasker.setInterval(updatesensorData, updatesensorData_interval * 1000, 0);
        // 设置定时器，按照设定的时间间隔执行updateEEPROMTask函数
        updateEEPROMTask.setInterval(updateEEPROM, updateEEPROM_interval * 1000 * 60, 0);
        // 设置定时器，每分钟执行一次sendEmail函数

        // 连接Wi-Fi
        // WiFi.begin(ssid.c_str(), password.c_str());
        // 输出正在连接到Wi-Fi的信息
        // Serial.print("连接到 Wi-Fi");
        // 循环等待Wi-Fi连接成功
        // while (WiFi.status() != WL_CONNECTED)
        // {
        //     // 输出等待提示
        //     Serial.print(".");
        //     // 等待300毫秒
        //     delay(300);
        // }
        // // 换行
        // Serial.println();
        // // 输出已连接的IP地址
        // Serial.print("Connected with IP: ");
        // Serial.println(WiFi.localIP());
        // // 换行
        // Serial.println();

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
    else
    {
        while (true)
        {
            server.handleClient();
        }
    }
}

void loop()
{

    server.handleClient();
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