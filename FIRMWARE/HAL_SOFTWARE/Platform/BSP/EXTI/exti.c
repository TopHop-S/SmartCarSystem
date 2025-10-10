/**
  ******************************************************************************
  * @file    exti.c
  * @author  TopHop
  * @date    2025/10/10
  * @version V1.0
  * @brief   板间 EXTI 驱动代码
  *
  ******************************************************************************
  * @attention
  * Copyright (c) 2025 TopHop. All rights reserved.
  * STM32F407VET6
  * 
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "./BSP/EXTI/exti.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"

/* Functions  ------------------------------------------------------------------*/
/** 
 * @brief KEY0 外部中断服务程序 
 * @retval 无
 */
void KEY0_INT_IRQHandler(void)
{
    /* 调用中断处理公用函数 清除KEY0所在中断线 的中断标志位 */
    HAL_GPIO_EXTI_IRQHandler(KEY0_INT_GPIO_PIN);
    /* HAL库默认先清中断在处理回调，退出时再清一次中断，避免按键抖动误触发 */
    __HAL_GPIO_EXTI_CLEAR_IT(KEY0_INT_GPIO_PIN);
}

/** 
 * @brief 外部中断回调函数 
 * @param GPIO_Pin: 中断引脚号 
 * @note 在 HAL 库中所有的外部中断服务函数都会调用此函数 
 * @retval 无 
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* 消抖，此处为了方便使用了延时函数，实际代码中禁止在中断服务函数中调用 
    任何 delay 之类的延时函数！！！ */
    delay_ms(20);
    switch (GPIO_Pin){
        case KEY0_INT_GPIO_PIN:
            
            LED0_TOGGLE();                  /* LED0 状态取反 */ 
            
            break;
        
        default:
            break;
    }
}

void extix_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    // key_init();
    gpio_init_struct.Pin = KEY0_INT_GPIO_PIN;           /* 选择KEY引脚 */
    gpio_init_struct.Mode = GPIO_MODE_IT_FALLING;       /* 下降沿触发 */
    gpio_init_struct.Pull = GPIO_PULLUP;                /* 上拉 */
    HAL_GPIO_Init(KEY0_INT_GPIO_PORT,&gpio_init_struct);    /* KEY0 引脚模式设置 */

    /* 设置中断优先级 */
    HAL_NVIC_SetPriority(KEY0_INT_IRQn,0,2);       /* 抢占优先级0, 子优先级2 */
    /* 使能中断 */
    HAL_NVIC_EnableIRQ(KEY0_INT_IRQn);             /* 使能 EXTI0中断 */
}




