/**
  ******************************************************************************
  * @file    led.c
  * @author  TopHop
  * @date    2025/10/09
  * @version V1.0
  * @brief   板间 LED 驱动代码
  *
  ******************************************************************************
  * @attention
  * Copyright (c) 2025 TopHop. All rights reserved.
  * STM32F407VET6
  * 
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "./BSP/LED/led.h"

/* Functions  ------------------------------------------------------------------*/
void led_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    LED0_GPIO_CLK_ENABLE();                             /* 使能LED端口时钟 */
    gpio_init_struct.Pin = LED0_GPIO_PIN;               /* 选择LED引脚 */
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;        /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;      /* 高速 */
    HAL_GPIO_Init(LED0_GPIO_PORT,&gpio_init_struct);    /* 初始化 */

    LED0_ON();
}


