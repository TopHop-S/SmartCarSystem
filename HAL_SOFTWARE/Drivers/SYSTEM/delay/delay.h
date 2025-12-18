/**
  ******************************************************************************
  * @file    delay.h
  * @author  TopHop
  * @brief   Header file for delay.c module
  ******************************************************************************
  * @attention
  *     
  ******************************************************************************
  */ 
 
#ifndef __DELAY_H__
#define __DELAY_H__

#include "sys/sys.h"


void delay_init(uint16_t sysclk);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);

#if (!SYS_SUPPORT_OS)
    void HAL_Delay(uint32_t Delay);
#endif

#endif  /* __DELAY_H__ */

