/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "dht11.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_lcd.h"
#include "bsp_ultrasonic.h"
#include "bsp_esp8266.h"
#include "bsp_delay.h"
#include "app_system.h"
#include "key.h"
void SystemClock_Config(void);

struct dev_Status status = {0};//untit16_t is_open=0  //0默认为开，1为关
void menu_Chinese(void) //显示中文界面函数
{
	for(int i=0;i<6;i++)
	{
		rectangle_2(0,0+i*20,50,16,BLUE);
	}
	show_any(0,0,BLACK,WHITE,yellow,32,16);//显示任意大小的汉字
	if(status.LED2_Status == 0)
		show_any(50,0,RED,GRAY0,close,16,16); //更新字模 关
	else 
		show_any(50,0,RED,GRAY0,open,16,16); //开
	show_any(0,20,BLACK,WHITE,blue,32,16);//显示任意大小的汉字
	if(status.LED3_Status == 0)
		show_any(50,20,RED,GRAY0,close,16,16); //更新字模 关
	else 
		show_any(50,20,RED,GRAY0,open,16,16); //开
	show_any(0,40,BLACK,WHITE,green,32,16);//显示任意大小的汉字
	if(status.LED4_Status == 0)
		show_any(50,40,RED,GRAY0,close,16,16); //更新字模 关
	else 
		show_any(50,40,RED,GRAY0,open,16,16); //开
	show_any(0,60,BLACK,WHITE,fan,32,16);//显示任意大小的汉字
	if(status.Fan_Status == 0)
		show_any(50,60,RED,GRAY0,close,16,16); //更新字模 关
	else 
		show_any(50,60,RED,GRAY0,open,16,16); //开
	show_any(0,78,BLACK,WHITE,ji,48,16);//显示任意大小的汉字
	if(status.Relay_Status == 0)
		show_any(50,78,RED,GRAY0,close,16,16); //更新字模 关
	else 
		show_any(50,78,RED,GRAY0,open,16,16); //开
	
	show_any(0,105,BLACK,WHITE,feng,48,16);//显示任意大小的汉字
	for(int i=0;i<2;i++)
	{
		rectangle_2(64,0+i*40,50,16,BLUE);
	}
	char buf[40];
	show_any(70,0,BLACK,WHITE,tem,32,16); // 温度
	sprintf(buf,"%d.%d",ucharT_data_H,ucharRH_data_L);
	Gui_DrawFont_GBK16(90,0,BLACK,WHITE,(uint8_t*)buf);
	
	show_any(70,40,BLACK,WHITE,wet,32,16); // 湿度 
	sprintf(buf,"%d.%d",ucharT_data_H,ucharRH_data_L);
	Gui_DrawFont_GBK16(90,40,BLACK,WHITE,(uint8_t*)buf);
	
}
void menu_English(void) //显示英文界面，直接调用字符串显示函数即可
{
	for(int i=0;i<6;i++)
	{
		rectangle_2(0,0+i*20,50,16,BLUE);
	}
	if(status.LED2_Status == 0)
		Gui_DrawFont_GBK16(0,0,BLACK,WHITE,(uint8_t*)"LEDY OFF");
	else
		Gui_DrawFont_GBK16(0,0,BLACK,WHITE,(uint8_t*)"LEDY ON ");
	if(status.LED3_Status == 0)
		Gui_DrawFont_GBK16(0,20,BLACK,WHITE,(uint8_t*)"LEDB OFF");
	else
		Gui_DrawFont_GBK16(0,20,BLACK,WHITE,(uint8_t*)"LEDB ON ");
	if(status.LED4_Status == 0)
		Gui_DrawFont_GBK16(0,40,BLACK,WHITE,(uint8_t*)"LEDG OFF");
	else
		Gui_DrawFont_GBK16(0,40,BLACK,WHITE,(uint8_t*)"LEDG ON ");
	if(status.Fan_Status == 0)
		Gui_DrawFont_GBK16(0,60,BLACK,WHITE,(uint8_t*)"Fan OFF");
	else
		Gui_DrawFont_GBK16(0,60,BLACK,WHITE,(uint8_t*)"Fan ON ");
	if(status.Relay_Status == 0)
		Gui_DrawFont_GBK16(0,78,BLACK,WHITE,(uint8_t*)"RELAY OFF");
	else
		Gui_DrawFont_GBK16(0,78,BLACK,WHITE,(uint8_t*)"RELAY ON ");
	
	Gui_DrawFont_GBK16(0,100,BLACK,WHITE,(uint8_t*)"BEEP");

	for(int i=0;i<2;i++)
	{
		rectangle_2(64,0+i*40,50,16,BLUE);
	}	
	char buf[40];
	Gui_DrawFont_GBK16(75,0,BLACK,WHITE,(uint8_t*)"TEMP");//温度
	sprintf(buf,"%d.%d",ucharT_data_H,ucharRH_data_L);
	Gui_DrawFont_GBK16(96,0,BLACK,WHITE,(uint8_t*)buf);
	
	Gui_DrawFont_GBK16(75,40,BLACK,WHITE,(uint8_t*)"HUM"); //湿度
	sprintf(buf,"%d.%d",ucharT_data_H,ucharRH_data_L);
	Gui_DrawFont_GBK16(96,40,BLACK,WHITE,(uint8_t*)buf);
}

void control_all(int line)//控制函数，line表示选取的行数
{
	switch(line)
	{
		case 1: //黄灯
			if(status.LED2_Status == 0)
			{
				LED2_YELLOW_ON;
				status.LED2_Status =1;
			} else{
				LED2_YELLOW_OFF;
				status.LED2_Status =0;
			}
			break;
		case 2: //蓝灯
			if(status.LED3_Status == 0)
			{
				LED3_BLUE_ON;
				status.LED3_Status =1;
			} else{
				LED3_BLUE_OFF;
				status.LED3_Status =0;
			}
			break;
		case 3: //绿灯
			if(status.LED4_Status == 0)
			{
				LED4_GREEN_ON;
				status.LED4_Status =1;
			} else{
				LED4_GREEN_OFF;
				status.LED4_Status =0;
			}
			break;
		case 4: //风扇
			if(status.Fan_Status == 0)
			{
				Fan_Init(); //风扇初始化
				FAN_ON;
				status.Fan_Status =1;
			} else{
				FAN_OFF;
				status.Fan_Status =0;
			}
			break;
		case 5: //继电器
			if(status.LED4_Status == 0)
			{
				RELAY_Init();//继电器初始化
				RELAY_ON;
				status.Relay_Status =1;
			} else{
				RELAY_OFF;
				status.Relay_Status =0;
			}
			break;
	}
	
/*	is_open = !is_open; //切换状态
	if(is_open == 1 )
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET); // 点亮黄灯	
		show_any(50,5,WHITE,RED,open,16,16); //更新字模
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET); // 熄灭黄灯
		show_any(50,5,WHITE,RED,close,16,16);  //更新字模
	}
	*/
 
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  // ---- 外设初始化：串口1为调试口，串口2接ESP8266 ----
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  // 启动 USART2 的 DMA 接收（否则 ESP8266 的回复永远收不到）
  HAL_UART_Receive_DMA(&huart2, Usart2type.UsartDMARecBuffer, USART_DMA_REC_SIE);
  MX_TIM1_Init();
  HAL_TIM_Base_Start_IT(&htim1);   // 1秒定时中断，产生上报标志

  delay_init(64);

  // ---- 通过ADC电压识别本板接的是哪种传感器/设备 ----
  BSP_Init();
  DisplayDeviceLogo();   // LCD 显示设备名称

  // ---- ESP8266：连WiFi -> 建立UDP -> 进入透传 ----
  ESP8266_Init();

  // ---- 主循环：网络状态机 + 每秒上报JSON/解析命令 ----
  while (1)
  {
    ESP_TASK();   // 处理AT指令状态机
    SEND_TASK();  // 上报传感器数据，同时解析上位机下发的命令
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage 
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks 
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

int fputc(int ch, FILE *f)
{
	#if DEBUG_Printf
		HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
	#endif
  return ch;
}


void Blink_Leds(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t ms)
{
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
  HAL_Delay(ms);
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
  HAL_Delay(ms);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
