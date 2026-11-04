#include "bsp_esp8266.h"
#include "usart.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bsp_lcd.h"
#include "bsp_softwareTimer.h"
#include "fs_protocol.h"
#include "app_system.h"
#include "bsp_beep.h"
#include "bsp_P9813.h"


static tsTimeType timeESP, timeSendData; // 软件定时器
teESP_TaskStatus ESP_TaskStatus;         // 模组任务状态
static teATCmdNum ATNextCmdNum;          // 下条指令
static teATCmdNum ATCurrentCmdNum;       // 当前指令
static teATCmdNum ATRecCmdNum;           // 接收指令
static uint8_t CurrentRty;               // 重发次数

#define WIFI_SSID "lyj"             // WiFi热点的SSID号（手机热点名）
#define WIFI_PASSWD "987654321"     // WiFi热点的密码

#define UDP_SER_IP "255.255.255.255" // IP地址

static char *LOCAL_WIFI_IP = NULL; // 本地IP地址

tsATCmds ATCmds[] = {
    {"AT\r\n", "OK", 500, NO_REC, 20},                                      // 查看ESP8266是否正常运行
    {"AT+CWMODE=1\r\n", "OK", 500, NO_REC, 20},                             // 设置为station模式
    {"AT+CWJAP=\"%s\",\"%s\"\r\n", "OK", 3000, NO_REC, 20},                 // 连接AP热点
    {"AT+CIFSR\r\n", "OK", 500, NO_REC, 5},                                 // 查询本地IP地址
		
    {"AT+CIPSTART=\"UDP\",\"%s\",20000,20000,0\r\n", "OK", 1000, NO_REC, 20}, 	//设置UDP服务端模式
		{"AT+CIPMUX=0\r\n", "OK", 500, NO_REC, 20},	// 配置WiFi为单链接模式
    {"AT+CIPMODE=1\r\n", "OK", 500, NO_REC, 20},                            // 开启透传模式
    {"AT+CIPSEND\r\n", "AT+CIPSEND\r\r\n\r\nOK\r\n\r\n>", 500, NO_REC, 20}, // 开始透传
		
		{"+++", "+++", 2000, NO_REC, 3},						// 关闭透传数据
    {"AT+CIPMODE=0\r\n", "OK", 500, NO_REC, 3},	// 关闭透传模式
    {"AT+CIPCLOSE\r\n", "OK", 500, NO_REC, 3}, 	// 断开连接
};

/**
 * 函数功能： 初始化 ESP8266-WiFi模组 相关变量
 * 参数：
 *      无
 * 返回值：
 *      无
 */
void ESP8266_Init(void)
{
  printf("ESP8266_Init...\r\n");
	
	Gui_DrawFont_GBK16(94, 32, BLACK, YELLOW, (uint8_t *)WIFI_SSID);
	
  ESP_TaskStatus = ESP_SEND;                        //进入发送模式
  ATCurrentCmdNum = AT;                             //当前指令
  ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1); //下一条指令
}
/**
 * 函数功能： ESP8266 发送AT指令数据
 * 参数：
 *      @ATCmdNum： AT指令枚举，与前面AT指令对应
 * 返回值：
 *      无
 */
static void ATSend(teATCmdNum ATCmdNum)
{
  memset(Usart2type.UsartRecBuffer, 0, USART_REC_SIE);
  ATCmds[ATCmdNum].ATStatus = NO_REC; //设置为 未接收
  ATRecCmdNum = ATCmdNum;
  char AT_SendBuff[100] = {0};

  // 连接AP热点
  if (ATCmdNum == AT_CWJAP)
  {
    sprintf(AT_SendBuff, ATCmds[ATCmdNum].ATSendStr, WIFI_SSID, WIFI_PASSWD);
    HAL_UART_Transmit(&huart2, (uint8_t *)AT_SendBuff, strlen(AT_SendBuff), 100);
    printf("SendDataBuff:%s\r\n", AT_SendBuff);
  }
  // 建立服务器
  else if (ATCmdNum == AT_CIPSTART)
  {
    sprintf(AT_SendBuff, ATCmds[ATCmdNum].ATSendStr, UDP_SER_IP);
    HAL_UART_Transmit(&huart2, (uint8_t *)AT_SendBuff, strlen(AT_SendBuff), 100);
    printf("SendDataBuff:%s\r\n", AT_SendBuff);
  }
  else
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)ATCmds[ATCmdNum].ATSendStr, strlen(ATCmds[ATCmdNum].ATSendStr), 0xff);
    printf("SendDataBuff:%s\r\n", ATCmds[ATCmdNum].ATSendStr);
  }

  //打开超时定时器
  setTime(&timeESP, ATCmds[ATCmdNum].TimeOut);
}
/**
 * 函数功能： ESP8266 接收数据
 * 参数：
 *      无
 * 返回值：
 *      无
 */
static void ATRecv(void)
{
  if (Usart2type.UsartRecFlag) // 串口接收到数据
  {
    printf("RecvStr: %s, Rev: [%s], len:%d\r\n",
           ATCmds[ATRecCmdNum].ATRecStr, Usart2type.UsartRecBuffer, Usart2type.UsartRecLen);

    if (strstr((const char *)Usart2type.UsartRecBuffer, ATCmds[ATRecCmdNum].ATRecStr) != NULL) //如果包含对应校验符
    {
      ATCmds[ATRecCmdNum].ATStatus = SUCCESS_REC; //设置为成功接收
    }

    // 如果是查询IP地址成功了
    if ((ATRecCmdNum == AT_CIFSR) && (ATCmds[AT_CIFSR].ATStatus == SUCCESS_REC))
    {
      char *pStrstr = NULL;
      char *pStrtok = NULL;
      if ((pStrstr = strstr((const char *)Usart2type.UsartRecBuffer, "+CIFSR:STAIP")) != NULL)
      {
        pStrtok = strtok(pStrstr, "\"");
        printf("========= pStrtok: %s\r\n", pStrtok);

        LOCAL_WIFI_IP = strtok(NULL, "\"");
        printf("========= Local WiFi IP: %s\r\n", LOCAL_WIFI_IP);
        Gui_DrawFont_GBK16(30, 48, BLACK, YELLOW, (uint8_t *)LOCAL_WIFI_IP);

        LED_NET_ON;

        pStrstr = NULL;
        pStrtok = NULL;
      }
    }
    Usart2type.UsartRecFlag = 0; //清空接收标志位
    Usart2type.UsartRecLen = 0;  //清空接收长度
    memset(Usart2type.UsartRecBuffer, 0, sizeof(Usart2type.UsartRecBuffer));
  }
}

/**
 * 函数功能： ESP8266 处理成功命令与失败命令
 * 参数：
 *      无
 * 返回值：
 *      无
 */
static void Rec_WaitAT(void)
{

  if (ATCmds[ATCurrentCmdNum].ATStatus == SUCCESS_REC) // 成功接收到数据包
  {
    switch (ATCurrentCmdNum)
    {
    case AT: // AT指令测试成功
      printf("\r\n AT success...\r\n");
      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CWMODE: // 设置WiFi工作模式
      printf("\r\n AT_CWMODE success...\r\n");
      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CWJAP: // 连接AP热点
      printf("\r\n AT_CWJAP success...\r\n");
      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CIFSR: // 查询本地IP地址
      printf("\r\n AT_CIFSR success...\r\n");

      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CIPSTART: // 建立连接
      printf("\r\n AT_CIPSTART success...\r\n");

      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CIPMODE_1: // 设置透传模式
      printf("\r\n AT_CIPMODE success...\r\n");
      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CIPSEND: // 进入透传模式成功

      printf("\r\n AT_CIPSEND success...\r\n");
      ESP_TaskStatus = ESP_IDIE;
			
			LED_NET_ON;
			
      setTime(&timeSendData, 500);
      break;

    default:
      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND; // 发送下一条命令
      break;
    }
  }

  else if (compareTime(&timeESP)) //还没收到预期数据  并且超时
  {
    printf("CurrentRty: %d, CmdNum: %d\r\n", CurrentRty, ATCurrentCmdNum);
		
		switch(ATCurrentCmdNum)
		{
			case AT_CWJAP:
				LED_NET_ON;
				HAL_Delay(100);
				LED_NET_OFF;
				LED_RECV_ON;
				HAL_Delay(50);
				LED_RECV_OFF;
				LED_SEND_ON;
				HAL_Delay(50);
				LED_SEND_OFF;
			break;
			default: 
			break;
		}
		
    ATCmds[ATCurrentCmdNum].ATStatus = TIME_OUT;
		LED_NET_OFF;
    if (CurrentRty > 0) //重发
    {
      CurrentRty--;
      printf("now:%d,trytimes:%d\r\n", ATCurrentCmdNum, CurrentRty);
      ATNextCmdNum = ATCurrentCmdNum; //下一条 还是当前命令  实现重发效果
      ESP_TaskStatus = ESP_SEND;
      return;
    }
    else //重发次数用完
    {
      ATCurrentCmdNum = AT;
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      return;
    }
  }
}

/**
 * 函数功能： ESP8266 任务的状态机
 * 参数：
 *      无
 * 返回值：
 *      无
 */
void ESP_TASK(void)
{
  while (1)
  {
    switch (ESP_TaskStatus)
    {
    case ESP_IDIE: // 空闲状态

      SEND_TASK(); // 空闲状态
      return;

    case ESP_SEND:                         // 发送状态 发送ESP8266的AT指令
      if (ATCurrentCmdNum != ATNextCmdNum) // 不是重发状态
      {
        CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum; //重发次数赋值
      }

      ATSend(ATCurrentCmdNum); //发送数据
      ESP_TaskStatus = ESP_WAIT;
      return;
    case ESP_WAIT: //等待数据状态 接收ESP8266的AT指令返回的数据

      ATRecv();     // 接收数据
      Rec_WaitAT(); //处理成功命令与失败命令
      return;
    default:
      // 其他
      return;
    }
  }
}

static char SendBuff[128] = {0};
static int SendLenth = 0;

// 本地报警阈值：超过就上报 alarm=1，上位机收到后触发人脸核验
#define ALARM_DISTANCE_CM 25
#define ALARM_TEMP_VALUE  30

/**
 * 函数功能： 把传感器数据打包成 JSON 字符串
 * 参数： buff 输出缓冲区
 * 返回值： JSON 长度，-1 表示本板没有需要上报的数据
 */
static int packet_json(char *buff)
{
  switch (global_Data.DeviceType)
  {
  case DEVICE_TYPE_TEMPHUMI: // 温湿度：{"dev":"temp_humi","temp":26,"humi":58,"alarm":0}
    return sprintf(buff, "{\"dev\":\"temp_humi\",\"temp\":%d,\"humi\":%d,\"alarm\":%d}",
                   global_Data.TempValue, global_Data.HumiValue,
                   (global_Data.TempValue > ALARM_TEMP_VALUE) ? 1 : 0);

  case DEVICE_TYPE_ULTRASONIC: // 超声波：{"dev":"ultrasonic","dist":120,"alarm":0}
    return sprintf(buff, "{\"dev\":\"ultrasonic\",\"dist\":%d,\"alarm\":%d}",
                   global_Data.Distance,
                   (global_Data.Distance < ALARM_DISTANCE_CM) ? 1 : 0);

  case DEVICE_TYPE_FAN:
    return sprintf(buff, "{\"dev\":\"fan\",\"state\":%d}",
                   (global_Data.SwitchValue == SWITCH_ON) ? 1 : 0);

  case DEVICE_TYPE_RELAY:
    return sprintf(buff, "{\"dev\":\"relay\",\"state\":%d}",
                   (global_Data.SwitchValue == SWITCH_ON) ? 1 : 0);

  case DEVICE_TYPE_BEEP:
    return sprintf(buff, "{\"dev\":\"beep\",\"state\":%d}",
                   (global_Data.SwitchValue == SWITCH_ON) ? 1 : 0);

  case DEVICE_TYPE_LAMP: // RGB 灯：{"dev":"lamp","r":255,"g":0,"b":0}
    return sprintf(buff, "{\"dev\":\"lamp\",\"r\":%d,\"g\":%d,\"b\":%d}",
                   global_Data.RedValue, global_Data.GreenValue, global_Data.BlueValue);

  case DEVICE_TYPE_INFRARED:
  case DEVICE_TYPE_FLAME:
  case DEVICE_TYPE_TOUCH:
    return sprintf(buff, "{\"dev\":\"%s\",\"state\":%d}",
                   (global_Data.DeviceType == DEVICE_TYPE_INFRARED) ? "infrared" :
                   (global_Data.DeviceType == DEVICE_TYPE_FLAME) ? "flame" : "touch",
                   (global_Data.SwitchValue == SWITCH_ON) ? 1 : 0);

  case DEVICE_TYPE_LIGHT:
    return sprintf(buff, "{\"dev\":\"light\",\"mv\":%d}", global_Data.ADCValue);

  case DEVICE_TYPE_GAS:
    return sprintf(buff, "{\"dev\":\"gas\",\"mv\":%d}", global_Data.ADCValue);

  default:
    return -1;
  }
}

/**
 * 函数功能： 解析上位机下发的 JSON 命令，例如 {"cmd":"fan","val":1}
 *            每块板子只处理自己对应的命令
 */
static void analysis_json(char *buff)
{
  char *p = NULL;

  if (strstr(buff, "\"cmd\":\"fan\"") != NULL)
  {
    if (global_Data.DeviceType == DEVICE_TYPE_FAN)
    {
      p = strstr(buff, "\"val\":");
      if (p != NULL && atoi(p + 6) == 1)
      {
        FAN_ON;
        global_Data.SwitchValue = SWITCH_ON;
      }
      else
      {
        FAN_OFF;
        global_Data.SwitchValue = SWITCH_OFF;
      }
      Blink_Leds(LED_RECV_GPIO_Port, LED_RECV_Pin, 100);
      printf("===== FAN CMD =====\r\n");
    }
  }
  else if (strstr(buff, "\"cmd\":\"relay\"") != NULL)
  {
    if (global_Data.DeviceType == DEVICE_TYPE_RELAY)
    {
      p = strstr(buff, "\"val\":");
      if (p != NULL && atoi(p + 6) == 1)
      {
        RELAY_ON;
        global_Data.SwitchValue = SWITCH_ON;
      }
      else
      {
        RELAY_OFF;
        global_Data.SwitchValue = SWITCH_OFF;
      }
      Blink_Leds(LED_RECV_GPIO_Port, LED_RECV_Pin, 100);
      printf("===== RELAY CMD =====\r\n");
    }
  }
  else if (strstr(buff, "\"cmd\":\"beep\"") != NULL)
  {
    if (global_Data.DeviceType == DEVICE_TYPE_BEEP)
    {
      p = strstr(buff, "\"val\":");
      if (p != NULL && atoi(p + 6) == 1)
      {
        set_Beep_Status(1);
        global_Data.SwitchValue = SWITCH_ON;
      }
      else
      {
        set_Beep_Status(0);
        global_Data.SwitchValue = SWITCH_OFF;
      }
      Blink_Leds(LED_RECV_GPIO_Port, LED_RECV_Pin, 100);
      printf("===== BEEP CMD =====\r\n");
    }
  }
  else if (strstr(buff, "\"cmd\":\"lamp\"") != NULL)
  {
    if (global_Data.DeviceType == DEVICE_TYPE_LAMP)
    {
      p = strstr(buff, "\"r\":");
      global_Data.RedValue = (p != NULL) ? (uint8_t)atoi(p + 4) : 0;
      p = strstr(buff, "\"g\":");
      global_Data.GreenValue = (p != NULL) ? (uint8_t)atoi(p + 4) : 0;
      p = strstr(buff, "\"b\":");
      global_Data.BlueValue = (p != NULL) ? (uint8_t)atoi(p + 4) : 0;

      rgb_setValue(global_Data.RedValue, global_Data.GreenValue, global_Data.BlueValue);
      Blink_Leds(LED_RECV_GPIO_Port, LED_RECV_Pin, 100);
      printf("===== LAMP CMD =====\r\n");
    }
  }
}

/**
 * 函数功能： ESP8266 发送数据任务
 * 参数：
 *      无
 * 返回值：
 *      无
 */
void SEND_TASK(void)
{
  if (ESP_TaskStatus == ESP_IDIE)
  {
    // 每隔一秒，上报一次数据
    if (global_Data.SendDataTime_Flag)
    {
      global_Data.SendDataTime_Flag = 0;

      Blink_Leds(LED_SEND_GPIO_Port, LED_SEND_Pin, 50);

      // 获取传感器数据
      get_SensorData();

      // LCD显示屏显示
      DisplayDeviceData();

      // 打包成 JSON 并发送（透传模式下直接写串口）
      memset(SendBuff, 0, sizeof(SendBuff));
      SendLenth = packet_json(SendBuff);
      if (SendLenth > 0)
      {
        HAL_UART_Transmit(&huart2, (uint8_t *)SendBuff, SendLenth, 100);
        HAL_UART_Transmit(&huart1, (uint8_t *)SendBuff, SendLenth, 100); // 调试口同步打印
      }
    }

    // 解析上位机下发的 JSON 命令
    if (Usart2type.UsartRecFlag)
    {
      HAL_UART_Transmit(&huart1, Usart2type.UsartRecBuffer, Usart2type.UsartRecLen, 100);

      analysis_json((char *)Usart2type.UsartRecBuffer);

      Usart2type.UsartRecFlag = 0; //清空接收标志位
      Usart2type.UsartRecLen = 0;  //清空接收长度
      memset(Usart2type.UsartRecBuffer, 0, sizeof(Usart2type.UsartRecBuffer));
    }
  }
}
