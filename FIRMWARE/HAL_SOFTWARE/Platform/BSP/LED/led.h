/**
  ******************************************************************************
  * @file    led.h
  * @author  TopHop
  * @date    2025/10/09
  * @version V1.0
  * @brief   Header file for sys.c module LED 驱动代码
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __LED_H__
#define __LED_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "./SYSTEM/sys/sys.h"

/* 引脚定义 */
#define LED0_GPIO_PORT  GPIOA
#define LED0_GPIO_PIN   GPIO_PIN_8
#define LED0_GPIO_CLK_ENABLE()  do{ __HAL_RCC_GPIOA_CLK_ENABLE();}while (0);    /* PA口时钟使能 */

/* LED端口定义 */
#define LED0_ON()   do{ HAL_GPIO_WritePin(LED0_GPIO_PORT,LED0_GPIO_PIN,GPIO_PIN_SET); }while (0);       /* 打开LED0 */
#define LED0_OFF()   do{ HAL_GPIO_WritePin(LED0_GPIO_PORT,LED0_GPIO_PIN,GPIO_PIN_RESET); }while (0);    /* 关闭LED0 */
#define LED0_TOGGLE()   do{ HAL_GPIO_TogglePin(LED0_GPIO_PORT,LED0_GPIO_PIN); }while (0);               /* 取反LED0 */

/* 外部接口函数 */
void led_init(void);                                                                            /* 初始化 */

#ifdef __cplusplus
}
#endif


#endif /* __LED_H__ */
