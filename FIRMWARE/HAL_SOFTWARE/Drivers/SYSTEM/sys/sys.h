/**
  ******************************************************************************
  * @file    sys.h
  * @author  TopHop
  * @brief   Header file for sys.c module
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYS_H__
#define __SYS_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Functions  ------------------------------------------------------------------*/

uint8_t sys_stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq); /* 配置系统时钟 */

#ifdef __cplusplus
}
#endif


#endif /* __SYS_H__ */
