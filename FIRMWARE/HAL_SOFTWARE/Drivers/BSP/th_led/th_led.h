/**
  ******************************************************************************
  * @file           : th_led.h
  * @brief          : Header for th_led.c file.
  *
  ******************************************************************************
  * @attention  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TH_LED_H__
#define __TH_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private defines -----------------------------------------------------------*/
// 定义明确的模式常量
#define LED_MODE_INPUT      0
#define LED_MODE_OUTPUT     1

#define TH_LED_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()    // LED IO 时钟使能

// GPIO配置
#define TH_LED_GPIO_PORT            GPIOA                           // LED IO TypeDef
#define TH_LED_GPIO_PIN             GPIO_PIN_8                      // LED IO PIN

/* Exported functions prototypes ---------------------------------------------*/
void th_led_init(uint8_t led_mode);

#ifdef __cplusplus
}
#endif

#endif /* __TH_LED_H__ */
