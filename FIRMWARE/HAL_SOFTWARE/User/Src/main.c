/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/EXTI/exti.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    HAL_Init();         /* 初始化HAL库 */
    sys_stm32_clock_init(336,8,2,7);    /* 配置系统时钟, 168Mhz */
    delay_init(168);    /* 初始化延时函数 */
    led_init();         /* 初始化LED */
    key_init();         /* 初始化KEY */

#if EXTI0_PHY_TEST
    extix_init();       /* 初始化EXTI */
#endif

#if UART_TEST
    usart_init(115200); /* 初始化串口 */
    uint32_t times = 0;
    uint16_t len = 0;
#endif

    LED0_OFF();
    
    while (1){

    #if LED_FLASHING_TEST
        LED0_TOGGLE();
        delay_ms(500);
    #endif

    #if KEY_PHY_TEST
        if(KEY0_PRES == key_scan()){
            LED0_ON();
        }else{
            LED0_OFF();
        }
    #endif

    #if EXTI0_PHY_TEST
        delay_ms(1000);
    #endif

    #if UART_TEST
        if(g_usart_rx_sta & 0x8000){        // 接收完成
            len = g_usart_rx_sta & 0x3FFF;  // 接收的字节数
            printf("\r\n 发送的消息为:\r\n");

            HAL_UART_Transmit(&uartx_handle, (uint8_t *)g_usart_rx_buf, len, 1000); // 发送数据
            
            printf("\r\n\r\n");             // 插入换行
            g_usart_rx_sta = 0;             // 清零准备下次接收
        }else{
            times++;

            if(times % 5000 == 0){
                printf("\r\n 串口1实验 \r\n");
            }

            if(times % 200 == 0){
                printf("请输入数据,以回车键结束 \r\n");
            }

            if(times % 50 == 0){
                LED0_TOGGLE();      /* 闪烁 LED,提示系统正在运行. */ 
            }
            
            delay_ms(10);
        }
    #endif

    }

    // LED0_OFF();         /* 程序结束 关闭LED0 */
}
