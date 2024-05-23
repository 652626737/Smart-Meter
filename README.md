# Smart-Meter
# 项目名称

## 简介
此项目旨在通过 ESP32 实现邮件客户端功能。项目包含多个模块文件，用于管理 WiFi 连接、EEPROM、文件系统等。

## 目录结构

├── src
│ ├── ESP_Mail_Client_Wrapper.cpp
│ ├── ESP_Mail_Client_Wrapper.h
│ ├── Button.h
│ ├── config.cpp
│ ├── config.h
│ ├── EEPROMManager.cpp
│ ├── EEPROMManager.h
│ ├── LittleFSManager.cpp
│ ├── LittleFSManager.h
│ ├── main.cpp
│ ├── NTPTimeManager.cpp
│ ├── NTPTimeManager.h
│ ├── SpiffsManager.cpp
│ ├── SpiffsManager.h
├── Smart Meter.code-workspace
├── main.txt


## 依赖项

在开始之前，请确保已经安装以下依赖项：

- [Arduino IDE](https://www.arduino.cc/en/software) 或 [PlatformIO](https://platformio.org/)
- [ESP32 开发板包](https://github.com/espressif/arduino-esp32)（适用于 Arduino IDE）
- WiFi 库（通常在 ESP32 开发环境中默认包含）

## 安装

### Arduino IDE

1. 打开 Arduino IDE。
2. 转到 `文件 -> 首选项`，在 `附加开发板管理器网址` 中添加以下链接：https://dl.espressif.com/dl/package_esp32_index.json
3. 打开 `工具 -> 开发板 -> 开发板管理器`，搜索并安装 `esp32`。
4. 确认 WiFi 库已经安装（通常在安装 ESP32 开发包时会默认包含）。

### PlatformIO

1. 安装 [VS Code](https://code.visualstudio.com/) 和 [PlatformIO 插件](https://platformio.org/install/ide?install=vscode)。
2. 创建一个新的 PlatformIO 项目，选择 `esp32dev` 开发板。
3. 在 `platformio.ini` 文件中添加所需的依赖项：
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    WiFi
```
### 使用
## 编译和上传
1.在 config.cpp 文件中编辑您的 WiFi 和邮箱配置：
```cpp
#include "config.h"

// Wi-Fi credentials
const char *ssid = "your_SSID";    // 替换为您的 Wi-Fi SSID
const char *password = "your_PASSWORD"; // 替换为您的 Wi-Fi 密码

// SMTP server details
const char *smtpServer = "smtp.qq.com";
const int smtpPort = 465;
const char *emailFrom = "your_email@example.com"; // 替换为您的邮箱地址
const char *passwordEmail = "your_email_password"; // 替换为您的邮箱密码
const char *emailTo = "recipient_email@example.com"; // 替换为收件人的邮箱地址

// NTP server details
const char *ntpServer = "ntp1.aliyun.com";
const long timeOffset = 0; // 时区偏移，以秒为单位
const long updateInterval = 60000; // 更新间隔，以毫秒为单位

// Authentication code
const char auth[] = "your_auth_code"; // 替换为您的认证码
```
2.打开 main.cpp 文件并确保包含 config.h：
```cpp
#include <WiFi.h>
#include "config.h"

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    // 检查连接状态
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("正在连接到WiFi...");
    }

    Serial.println("WiFi 已连接");
}

void loop() {
    // 主要代码放在这里
}
```
3.选择正确的开发板和端口。

4.点击上传按钮将代码上传到开发板。