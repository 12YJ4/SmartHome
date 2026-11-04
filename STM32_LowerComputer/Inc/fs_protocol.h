#ifndef __FS_PROTOCOL_H__
#define __FS_PROTOCOL_H__

#include "main.h"

// 21协议 每一位的含义
#define PROTOCOL_HEAD_OFF 0            // 数据标识
#define PROTOCOL_DATA_LEN_OFF 1        // 数据域-数据长度
#define PROTOCOL_DATA_OFFSET_OFF 2     // 数据域-数据偏移
#define PROTOCOL_DATA_TYPE_OFF 3       // 数据类型
#define PROTOCOL_NETWORK_TYPE_OFF 4    // 网络类型
#define PROTOCOL_SENSOR_ID_H_OFF 5     // 传感器设备地址-高位
#define PROTOCOL_SENSOR_ID_L_OFF 6     // 传感器设备地址-低位
#define PROTOCOL_SENSOR_TYPE_OFF 7     // 传感器类型
#define PROTOCOL_BATTERY_PERCENT_OFF 8 // 电池电量
#define PROTOCOL_VALID_DATA_OFF 9      // 数据域

#define FSP_PACKET_HEAD 0x21                          // 数据协议头
#define FSP_DATA_OFFSET_BYTES PROTOCOL_VALID_DATA_OFF // 数据偏移字节

#define FSP_UPLOAD_DATA 0x00 // 上报数据
#define FSP_CONTROL_CMD 0x01 // 下发命令
#define FSP_CRC_BYTES 1      // CRC校验位数据长度

#define SWITCH_ON '1'  // 开
#define SWITCH_OFF '0' // 关

#define FSP_WIFI 'W'           // 网络类型
// 传感器的设备类型
typedef enum {
  DEVICE_TYPE_FAN = 0x66, // 风扇
  DEVICE_TYPE_LIGHT = 0x4C, // 光照传感器
  DEVICE_TYPE_TEMPHUMI = 0x54, // 温湿度
  DEVICE_TYPE_GAS = 0x47,  // 可燃气体

  DEVICE_TYPE_PHOTOACTOR = 0x50, // 光电
  DEVICE_TYPE_LAMP = 0x6C,  // 电灯
  DEVICE_TYPE_RELAY = 0x72,  // 继电器
  DEVICE_TYPE_BEEP = 0x62,  // 蜂鸣器

  DEVICE_TYPE_INFRARED = 0x49,  // 人体红外传感器
  DEVICE_TYPE_ULTRASONIC = 0x55, // 超声波传感器
  DEVICE_TYPE_TOUCH = 0x34, // 触摸传感器 触摸 'T' - 0x20
  DEVICE_TYPE_FLAME = 0x26,  // 火焰  'F' - 0x20

  DEVICE_TYPE_ERR = 0xFF,  // 识别错误的传感器
} FSP_Device_Type_t;

// 传感器的设备地址
typedef enum {
  DEVICE_ADDR_FAN = 0x402C, // 风扇
  DEVICE_ADDR_LIGHT = 0x002A, // 光照传感器
  DEVICE_ADDR_TEMPHUMI = 0x002B, // 温湿度
  DEVICE_ADDR_GAS = 0x002D,  // 可燃气体

  DEVICE_ADDR_PHOTOACTOR = 0x002F, // 光电
  DEVICE_ADDR_LAMP = 0x402D,  // 电灯
  DEVICE_ADDR_RELAY = 0x402E,  // 继电器
  DEVICE_ADDR_BEEP = 0x402F,  // 蜂鸣器

  DEVICE_ADDR_INFRARED = 0x0032,  // 人体红外传感器
  DEVICE_ADDR_ULTRASONIC = 0x0033, // 超声波传感器
  DEVICE_ADDR_TOUCH = 0x0034, // 触摸传感器 
  DEVICE_ADDR_FLAME = 0x002E,  // 火焰 

} FSP_Device_Addr_t;


/**
 * @brief  组数据包
 * @note
 * @param  PacketBuff: 数据缓冲区
 * @param  SensorAddress: 传感器设备地址
 * @param  ValidData: 有效数据
 * @param  ValidLen: 有效数据长度
 * @retval 数据包的长度
 */
int packet_data(uint8_t *PacketBuff, uint16_t SensorAddress, uint8_t SensorType, uint8_t *ValidData, uint8_t ValidLen);
/**
 * @brief  解析数据包
 * @note
 * @param  PacketBuff: 数据缓冲区
 * @param  PacketLen: 数据缓冲区的长度
 * @retval None
 */
int analysis_Packet(uint8_t *PacketBuff, uint16_t PacketLen);

#endif
