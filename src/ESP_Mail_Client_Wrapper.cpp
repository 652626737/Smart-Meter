#include <WiFi.h>
#include "ESP_Mail_Client_Wrapper.h"


ESP_Mail_Client_Wrapper *ESP_Mail_Client_Wrapper::instance = nullptr;

ESP_Mail_Client_Wrapper::ESP_Mail_Client_Wrapper(const String &smtpServer, 
int smtpPort, const String &emailFrom, const String &passwordEmail, const String &emailTo)
{
    instance = this;
    MailClient.networkReconnect(true);

    smtp.debug(1);

    smtp.callback(smtpCallbackStatic);

    config.server.host_name = smtpServer.c_str();
    config.server.port = smtpPort;
    config.login.email = emailFrom.c_str();
    config.login.password = passwordEmail.c_str();
    config.login.user_domain = F("127.0.0.1");

    config.time.ntp_server = F("cn.ntp.org.cn");
    config.time.gmt_offset = 8;
    config.time.day_light_offset = 0;

    // 保存 emailTo 到成员变量中
    this->emailFrom = emailFrom;
    this->emailTo = emailTo;

}

void ESP_Mail_Client_Wrapper::send_mail(const String &subject,const String &additionalContent)
{

    
    SMTP_Message message;
    
    message.sender.name = F("Smart Meter");
    message.sender.email = emailFrom.c_str();
    message.subject = F(subject);
    message.addRecipient(F(emailTo.c_str()), emailTo.c_str());
    message.text.flowed = true;
    message.text.content += additionalContent;
    message.text.charSet = F("UTF-8");
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_8bit;
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
    message.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));

    if (!smtp.connect(&config))
    {
        MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
        return;
    }

    if (!smtp.isLoggedIn())
    {
        Serial.println("Not yet logged in.");
    }
    else
    {
        if (smtp.isAuthenticated())
            Serial.println("Successfully logged in.");
        else
            Serial.println("Connected with no Auth.");
    }

    if (!MailClient.sendMail(&smtp, &message))
        MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

    MailClient.printf("Free Heap: %d\n", MailClient.getFreeHeap());
}

void ESP_Mail_Client_Wrapper::smtpCallback(SMTP_Status status)
{
    Serial.println(status.info());

    if (status.success())
    {
        Serial.println("----------------");
        MailClient.printf("Message sent success: %d\n", status.completedCount());
        MailClient.printf("Message sent failed: %d\n", status.failedCount());
        Serial.println("----------------\n");

        for (size_t i = 0; i < smtp.sendingResult.size(); i++)
        {
            SMTP_Result result = smtp.sendingResult.getItem(i);

            MailClient.printf("Message No: %d\n", i + 1);
            MailClient.printf("Status: %s\n", result.completed ? "success" : "failed");
            MailClient.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
            MailClient.printf("Recipient: %s\n", result.recipients.c_str());
            MailClient.printf("Subject: %s\n", result.subject.c_str());
        }
        Serial.println("----------------\n");

        smtp.sendingResult.clear();
    }
}

void ESP_Mail_Client_Wrapper::smtpCallbackStatic(SMTP_Status status)
{
    if (instance)
    {
        instance->smtpCallback(status);
    }
}