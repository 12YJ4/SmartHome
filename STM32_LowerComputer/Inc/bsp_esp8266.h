#ifndef __ESP_WIFI_H__
#define __ESP_WIFI_H__

#include "main.h"

//AT指令状态机
typedef enum
{
    ESP_IDIE = 0,
    ESP_SEND,
    ESP_WAIT,
    ESP_ACCESS
} teESP_TaskStatus;
//AT指令枚举，与前面AT指令对应
typedef enum
{
	AT,
	AT_CWMODE,
  AT_CWJAP,
	AT_CIFSR,
	AT_CIPSTART,
	AT_CIPMUX,
	AT_CIPMODE_1,
	AT_CIPSEND,
	AT_QUITSEND,
	AT_CIPMODE_0,
  AT_CIPCLOSE,
} teATCmdNum;
// AT指令的状态
typedef enum
{
    SUCCESS_REC = 0,    // 接收成功
    TIME_OUT,           // 接收超时
    NO_REC              // 无返回
} teATStatus;
//AT指令结构体  5个字段
typedef struct
{
    char *ATSendStr;
    char *ATRecStr;
    double TimeOut;       //等待超时时间 （单位：ms）
    teATStatus ATStatus;    //指令状态
    uint8_t RtyNum;         //重发次数
} tsATCmds;


extern teESP_TaskStatus ESP_TaskStatus; // 模组任务状态

void ESP8266_Init(void);
void ESP_TASK(void);
void SEND_TASK(void);


#endif



