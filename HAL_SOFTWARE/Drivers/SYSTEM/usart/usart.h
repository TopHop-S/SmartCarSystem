/**
  ******************************************************************************
  * @file    usart.h
  * @author  TopHop
  * @brief   Header file for usart.c module
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"

/* define ------------------------------------------------------------------*/
#define USART_REC_LEN               200             // USART receive buffer length
#define USART_RX_OVER_BIT           (1<<15)         // USART receive over flag bit
#define USART_RX_0X0D_BIT           (1<<14)         // USART receive 0x0D('/r/n') flag bit
#define USART_RX_DATA_LEN_BIT       (0X3FFF)        // USART receive data length bit mask

// debug
#define TH_DEBUG_UART_BASE          USART2          // debug 
#define TH_DEBUG_UART_CLK_ENABLE()  __HAL_RCC_USART2_CLK_ENABLE();      // 

/**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
*/
#define TH_DEBUG_GPIO_TX_TYPE           GPIOA                               // DEBUG GPIO 
#define TH_DEBUG_GPIO_RX_TYPE           GPIOA                               // DEBUG GPIO 
#define TH_DEBUG_GPIO_TXCLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE();      // DEBUG GPIO 
#define TH_DEBUG_GPIO_RXCLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE();      // DEBUG GPIO 
#define TH_DEBUG_UART_TX_PIN            GPIO_PIN_2                          // DEBUG 
#define TH_DEBUG_UART_RX_PIN            GPIO_PIN_3                          // DEBUG 
#define TH_DEBUG_UART_AF                GPIO_AF7_USART2                     // DEBUG 
#define TH_DEBUG_IRQn_TYPE              USART2_IRQn                         // debug
#define TH_DEBUG_UART_IRQHandler        USART2_IRQHandler                   // debug

/* extern ------------------------------------------------------------------*/

extern uint8_t  th_debug_uart_rx_buf[USART_REC_LEN];    // debug 
extern uint16_t th_debug_uart_rx_sta;                   // debug 

/* Functions  ------------------------------------------------------------------*/

void th_debug_uart_init(uint32_t bound);

#ifdef __cplusplus
}
#endif


#endif /* __SYS_H__ */



