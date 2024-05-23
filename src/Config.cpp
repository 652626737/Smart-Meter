#include "Config.h"

// Wi-Fi credentials
const char *ssid = "OpenWrt2.4";    // 替换为您的 Wi-Fi SSID
const char *password = "12340987."; // 替换为您的 Wi-Fi 密码

// SMTP server details
const char *smtpServer = "smtp.qq.com";
const int smtpPort = 465;
const char *emailFrom = "652626737@qq.com"; // 替换为您的邮箱地址

/**
 * ""; // 替换为您的邮箱密码
 * ""; // 替换为您的邮箱密码2024-05-21 14:31:15
 * ""; // 替换为您的邮箱密码2024-05-22 23:08:17
 */
const char *passwordEmail = ""; // 替换为您的邮箱密码
const char *emailTo = "13732546017@163.com";    // 替换为收件人的邮箱地址

const char *ntpServer = "ntp1.aliyun.com";
const long timeOffset = 0;               // 时区偏移，以秒为单位
const long updateInterval = 60000; // 更新间隔，以毫秒为单位

const char auth[] = ""; // 替换为您的认证码
