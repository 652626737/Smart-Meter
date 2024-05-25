#ifndef ESP_MAIL_CLIENT_WRAPPER_H
#define ESP_MAIL_CLIENT_WRAPPER_H

#include <Arduino.h>
#include <ESP_Mail_Client.h>

class ESP_Mail_Client_Wrapper
{
public:
    ESP_Mail_Client_Wrapper(const String &smtpServer , int smtpPort, const String &emailFrom , const String &passwordEmail , const String &emailTo ); // 构造函数

    void send_mail(const String &subject,const String &additionalContent); // 发送电子邮件的函数

private:
    SMTPSession smtp;
    Session_Config config;
    
    String emailFrom;
    String emailTo;

    static void smtpCallbackStatic(SMTP_Status status);
    static ESP_Mail_Client_Wrapper *instance;
    void smtpCallback(SMTP_Status status);

};

#endif // ESP_MAIL_CLIENT_WRAPPER_H
