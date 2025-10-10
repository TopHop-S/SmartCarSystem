/**
  ******************************************************************************
  * @file    key.c
  * @author  TopHop
  * @date    2025/10/10
  * @version V1.0
  * @brief   板间 KEY 驱动代码
  *
  ******************************************************************************
  * @attention
  * Copyright (c) 2025 TopHop. All rights reserved.
  * STM32F407VET6
  * 
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "./BSP/KEY/key.h"
#include "./SYSTEM/delay/delay.h"

#define KEY0_PRESS      0       /* KEY0按下状态 */
#define KEY0_RELEASE    1       /* KEY0松开状态 */

/* Functions  ------------------------------------------------------------------*/
void key_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    KEY0_GPIO_CLK_ENABLE();                             /* 使能KEY端口时钟 */
    gpio_init_struct.Pin = KEY0_GPIO_PIN;               /* 选择KEY引脚 */
    gpio_init_struct.Mode = GPIO_MODE_INPUT;            /* 输入 */
    gpio_init_struct.Pull = GPIO_PULLUP;                /* 上拉-key外接是地 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;      /* 高速 */
    HAL_GPIO_Init(KEY0_GPIO_PORT,&gpio_init_struct);    /* KEY0 引脚模式设置 */

}

/** 
 * @brief 按键扫描函数 
 * @note 该函数有响应优先级(同时按下多个按键): WKUP > KEY2 > KEY1 > KEY0!! 
 * @arg 支持连续按
 * @retval 物理键值, 定义如下: 
 * KEY0_PRES, 1, KEY0 按下 
 * KEY1_PRES, 2, KEY1 按下 
 * KEY2_PRES, 3, KEY2 按下 
 */
uint8_t key_scan(void)
{
    uint8_t key_up = 1;     /* 按键按松开标志 */
    uint8_t key_value = KEY_NONE;
    
    
    if(key_up){
        delay_ms(10);       /* 按键消抖 */
        key_up = 0;
        if(KEY0_STATUS == KEY0_PRESS){
            key_value = KEY0_PRES;
        }
    }
    return key_value;
}


