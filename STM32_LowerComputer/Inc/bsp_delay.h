#ifndef __BSP_DELAY_H__
#define __BSP_DELAY_H__

#include "main.h"

void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);
void delay_init(uint8_t SYSCLK);


#endif


