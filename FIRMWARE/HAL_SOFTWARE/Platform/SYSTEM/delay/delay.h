/**
  ******************************************************************************
  * @file    delay.h
  * @author  TopHop
  * @date    2025/10/09
  * @version V1.0
  * @brief   使用SysTick的普通计数模式对延迟进行管理(支持freertos和ucosii)
  * 提供delay_init初始化函数， delay_us和delay_ms等延时函数
  *
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DELAY_H__
#define __DELAY_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "./SYSTEM/sys/sys.h"

/* Functions  ------------------------------------------------------------------*/
void delay_init(uint16_t sysclk);           /* 初始化延迟函数 */
void delay_ms(uint32_t nms);                /* 延时nms */
void delay_us(uint32_t nus);                /* 延时nus */

#if (!SYS_SUPPORT_OS)                       /* 没有使用Systick中断 */
    void HAL_Delay(uint32_t Delay);         /* HAL库的延时函数，SDIO等需要用到 */
#endif

#ifdef __cplusplus
}
#endif


#endif /* __DELAY_H__ */
