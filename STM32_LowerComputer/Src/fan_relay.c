#include "gpio.h"
//继电器的初始化函数： 如果在MX_GPIO_Init()中已完成初始化，此函数不需要使用
void RELAY_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();   //（1） GPIOB 端口B RCC时钟使能（就可以下面的修改操作）

  /*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); //(继电器)写 低电平-关闭 	
  //（2） 配置GPIOB相关引脚输出方向
  /*Configure GPIO pins : PBPin PBPin PBPin */
  GPIO_InitStruct.Pin = GPIO_PIN_7;  //  (继电器)引脚PB7
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  //output 输出   pp 推挽
  GPIO_InitStruct.Pull = GPIO_PULLUP;    //pull up  上拉电阻
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  // low  低速度
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  //将上面的设置 初始化实现
}
//风扇的初始化函数： 如果在MX_GPIO_Init()中已完成初始化，此函数不需要使用
void Fan_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();   //（1） GPIOB 端口B RCC时钟使能（就可以下面的修改操作）

  /*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET); //(风扇)写 高电平-关闭 	
  //（2） 配置GPIOB相关引脚输出方向
  /*Configure GPIO pins : PBPin PBPin PBPin */
  GPIO_InitStruct.Pin = GPIO_PIN_7;  // (风扇)引脚PB7
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  //output 输出   pp 推挽
  GPIO_InitStruct.Pull = GPIO_PULLUP;    //pull up  上拉电阻
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  // low  低速度
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  //将上面的设置 初始化实现
}

