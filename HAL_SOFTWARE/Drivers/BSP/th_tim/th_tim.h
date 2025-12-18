/**
  ******************************************************************************
  * @file           : th_tim.h
  * @brief          : Header for th_tim.c file.
  *
  ******************************************************************************
  * @attention  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TH_TIM_H__
#define __TH_TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private defines -----------------------------------------------------------*/
/**** PWM1配置 ****/
// 获取 TIM1 时钟频率（考虑 APB2 prescaler）
static inline uint32_t TH_PWM1_TIM_CLKFREQ(void)
{
    uint32_t _pclk = HAL_RCC_GetPCLK2Freq();
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_CFGR_PPRE2_DIV1) {
        _pclk *= 2U;
    }
    return _pclk;
}

#define TH_PWM1_TIM_BASE      				TIM1							// PWM定时器选择
// 时钟配置
#define TH_PWM1_TIM_CLK_ENABLE()            __HAL_RCC_TIM1_CLK_ENABLE();    // 使能定时器时钟
#define TH_PWM1_TIM_CHANNEL_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE();   // 使能通道IO时钟
// GPIO配置
#define TH_PWM1_GPIO_PORT                   GPIOA                           // PWM通道GPIO端口
#define TH_PWM1_GPIO_PIN                    GPIO_PIN_11                     // PWM通道GPIO引脚
#define TH_PWM1_GPIO_AF                     GPIO_AF1_TIM1                   // PWM通道GPIO复用功能

#define TH_PWM1_TIM_CHY                     TIM_CHANNEL_4                   // PWM通道选择
#define TH_PWM1_TIM_CHY_CCRX                TIM1->CCR4                      // PWM通道比较寄存器
#define TH_PWM1_TIM_IRQn                    TIM1_UP_TIM10_IRQn              // PWM定时器中断号
#define TH_PWM1_TIM_IRQHandler              TIM1_UP_TIM10_IRQHandler        // PWM定时器中断服务函数名称

/* Exported functions prototypes ---------------------------------------------*/

void th_tim_pwm1_chy_init(uint32_t psc, uint32_t arr);

int set_th_pwm1_freq(uint32_t freq_hz);

void set_th_pwm1_duty(uint32_t duty);

uint32_t get_th_pwm1_freq(void);

uint32_t get_th_pwm1_duty(void);

void th_key_scan_tim_init(void);




#ifdef __cplusplus
}
#endif

#endif /* __TH_TIM_H__ */
