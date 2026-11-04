#ifndef __DHT11_H__
#define __DHT11_H__

#include "main.h"
extern uint8_t ucharT_data_H,ucharT_data_L,ucharRH_data_H,ucharRH_data_L,ucharcheckdata;
uint8_t DHT11_Read_Data();
uint8_t FS_DHT11_Init(void);
void delay_init(uint8_t SYSCLK);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);
#endif


