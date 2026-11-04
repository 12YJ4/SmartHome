#ifndef __APP_SYSTEM_H__
#define __APP_SYSTEM_H__

#include <stdint.h>
#include "fs_protocol.h"

typedef enum ADC_INx
{
  ADC_IN0 = 0, //电池电量
  ADC_IN1,     // 五向按键
               //    ADC_IN2,
               //    ADC_IN3,
  ADC_IN4,     //传感器AD值
  ADC_IN5,     //传感器的类型
  ADC_NUM      // ADC转化的个数
} ADC_INx;


// 传感器模块的AD理论值 mv
typedef enum {
  DEVICE_ADC_FAN = 2100, // 风扇
  DEVICE_ADC_LIGHT = 61, // 光照传感器
  DEVICE_ADC_TEMPHUMI = 180, // 温湿度
  DEVICE_ADC_GAS = 2235,  // 可燃气体

  DEVICE_ADC_PHOTOACTOR = 420, // 光电
  DEVICE_ADC_LAMP = 540,  // 电灯
  DEVICE_ADC_RELAY = 660,  // 继电器
  DEVICE_ADC_BEEP = 780,  // 蜂鸣器

  DEVICE_ADC_INFRARED = 1140,  // 人体红外传感器
  DEVICE_ADC_ULTRASONIC = 1260, // 超声波传感器
  DEVICE_ADC_TOUCH = 1380, // 触摸传感器 触摸 'T' - 0x20
  DEVICE_ADC_FLAME = 300,  // 火焰  'F' - 0x20

  DEVICE_ADC_ERR = 0xFF,  // 识别错误的传感器
} FSP_Device_ADC_t;

#define ADC_ERRORS 60
#define ADCCOMP(c, x, y) (((x <= c) && (c <= y)) ? 1 : 0)

// 全局参数和系统配置
typedef struct __Global_Data__
{
  FSP_Device_Type_t DeviceType; // 传感器模块的类型
  FSP_Device_Addr_t DeviceAddr;  // 传感器的设备地址

  uint8_t SwitchValue;  // 开关量数据
  uint8_t RedValue, GreenValue, BlueValue;  // RGB灯数据

  uint8_t TempValue, HumiValue;//  温湿度的整数数据
  uint8_t PowerValue;  // 电池剩余电量
  uint8_t SendDataTime_Flag;  // 定时器中断
  
  uint16_t ADCValue;  // AD类数据
  uint16_t Distance;  // 超声波检测距离数据
} Global_Data_t;
extern Global_Data_t global_Data;

// 板级支持包初始化
void BSP_Init(void);
// 显示屏界面初始化
void DisplayDeviceLogo(void);
// 获取传感器的数据
void get_SensorData(void);
// LCD显示传感器数据
void DisplayDeviceData(void);

#endif
