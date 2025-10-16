/**
  ******************************************************************************
  * @file    usart.h
  * @author  TopHop
  * @date    2025/10/14
  * @version V1.0
  * @brief   Header file for usart.c module 串口通信 驱动代码
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "./SYSTEM/sys/sys.h"
#include "stdio.h"

/* 串口1的发送引脚GPIO */
#define USART_TX_GPIO_PORT      GPIOA
#define USART_TX_GPIO_PIN       GPIO_PIN_9
#define USART_TX_GPIO_AF        GPIO_AF7_USART1
#define USART_TX_GPIO_CLK_ENABLE()  do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)

/* 串口1的接收引脚GPIO */
#define USART_RX_GPIO_PORT      GPIOA
#define USART_RX_GPIO_PIN       GPIO_PIN_10
#define USART_RX_GPIO_AF        GPIO_AF7_USART1
#define USART_RX_GPIO_CLK_ENABLE()  do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)

/* 串口1中断 */
#define USART_UX                USART1
#define USART_UX_IRQn           USART1_IRQn
#define USART_UX_IRQHandler     USART1_IRQHandler   // 中断服务函数

/* 串口1的时钟使能 */
#define USART_UX_CLK_ENABLE()   do{ __HAL_RCC_USART1_CLK_ENABLE(); }while(0)

/* 自主定义 */
#define USART_REC_LEN       100     /* 定义最大接收字节数 100 */
#define USART_EN_RX         1       /* 使能接收功能 1使能 0禁止 */
#define RXBUFFERSIZE        1       /* 接收缓冲区大小 1个字节 */


extern uint16_t g_usart_rx_sta;
extern UART_HandleTypeDef uartx_handle;
extern uint8_t g_usart_rx_buf[USART_REC_LEN];

/* 外部接口函数 */
void usart_init(uint32_t baudrate);     /* 初始化 */

#ifdef __cplusplus
}
#endif


#endif /* __USART_H__ */
