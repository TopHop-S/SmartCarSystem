/**
  ******************************************************************************
  * @file    exti.h
  * @author  TopHop
  * @date    2025/10/10
  * @version V1.0
  * @brief   Header file for exti.c module EXTI 驱动代码
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __EXTI_H__
#define __EXTI_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "./SYSTEM/sys/sys.h"

/* 引脚 和 中断编号 & 中断服务函数 定义 */
#define KEY0_INT_GPIO_PORT      GPIOA
#define KEY0_INT_GPIO_PIN       GPIO_PIN_0

/* 物理按键KEY0 PA口时钟使能 */
#define KEY0_INT_GPIO_CLK_ENABLE()  do{ __HAL_RCC_GPIOA_CLK_ENABLE();}while (0);
#define KEY0_INT_IRQn               EXTI0_IRQn          /* EXTI0中断 */
#define KEY0_INT_IRQHandler         EXTI0_IRQHandler    /* EXTI0中断服务函数 startup_stm32f407xx.s 中写好了 */

/* 外部接口函数 */
void extix_init(void);    /* 初始化 */

#ifdef __cplusplus
}
#endif


#endif /* __EXTI_H__ */
