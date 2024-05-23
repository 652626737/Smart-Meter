#ifndef CONFIG_H
#define CONFIG_H

// Wi-Fi credentials
extern const char *ssid;     // 替换为您的 Wi-Fi SSID
extern const char *password; // 替换为您的 Wi-Fi 密码

// SMTP server details
extern const char *smtpServer;
extern const int smtpPort;
extern const char *emailFrom;     // 替换为您的邮箱地址
extern const char *passwordEmail; // 替换为您的邮箱密码
extern const char *emailTo;       // 替换为收件人的邮箱地址
extern const char *subject;
extern const char *emailMessage;

extern const char *ntpServer;        // NTP server
extern const char auth[];

extern const long timeOffset;           // NTP server
extern const long updateInterval;        // NTP server

#endif
