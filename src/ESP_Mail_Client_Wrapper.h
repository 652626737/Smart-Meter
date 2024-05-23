#ifndef ESP_MAIL_CLIENT_WRAPPER_H
#define ESP_MAIL_CLIENT_WRAPPER_H

#include <Arduino.h>
#include <ESP_Mail_Client.h>

class ESP_Mail_Client_Wrapper
{
public:
    ESP_Mail_Client_Wrapper(); // 构造函数

    void send_mail(const String &additionalContent,const String &subject); // 发送电子邮件的函数

private:
    SMTPSession smtp;
    Session_Config config;
    
    static void smtpCallbackStatic(SMTP_Status status);
    static ESP_Mail_Client_Wrapper *instance;
    void smtpCallback(SMTP_Status status);
};

#endif // ESP_MAIL_CLIENT_WRAPPER_H
