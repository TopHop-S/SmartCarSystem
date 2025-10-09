/**
  ******************************************************************************
  * @file    delay.c
  * @author  TopHop
  * @date    2025/10/09
  * @version V1.0
  * @brief   使用SysTick的普通计数模式对延迟进行管理(支持freertos和ucosii)，提供ms和us级延时
  *
  ******************************************************************************
  * @attention
  * Copyright (c) 2025 TopHop. All rights reserved.
  * STM32F407VET6
  * 
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "./SYSTEM/delay/delay.h"

/* Static ------------------------------------------------------------------*/
static uint32_t g_fac_us = 0;   /* us延时倍乘数 */


/* 如果SYS_SUPPORT_OS定义了,说明要支持OS了(不限于UCOS) */
#if SYS_SUPPORT_OS

#include "os.h"

#endif

/* Functions  ------------------------------------------------------------------*/
/** 
 * @brief 初始化延迟函数  
 * @param sysclk: 系统时钟频率, 即 CPU频率, 168MHz 
 * @retval 无
 */
void delay_init(uint16_t sysclk)
{
#if SYS_SUPPORT_OS      /* 如果需要支持 OS */
    uint32_t reload;
#endif

    g_fac_us = sysclk;
    /* 是由于在 HAL_Init中已对 systick做了配置，所以这里无需重新配置 */

#if SYS_SUPPORT_OS
    reload = sysclk;                            /* 每秒钟的计数次数 单位为 M */
    reload *= 1000000 / delay_ostickspersec;    /* 根据delay_ostickspersec设定溢出时间,reload为24位
                                                 * 寄存器,最大值:16777216,在168M下,约合0.09986s左右
                                                 */
    g_fac_us = 1000 / delay_ostickspersec;      /* 代表 OS可以延时的最少单位 */
    SysTick->CTRL |= 1 << 1;                    /* 开启 SYSTICK中断 */
    SysTick->LOAD = reload;                     /* 每 1/delay_ostickspersec秒中断一次 */
    SysTick->CTRL |= 1 << 0;                    /* SYSTICK开始计数 */
#endif

}

/**
 * @brief 延时nus微秒
 * @param nus:要延时的微秒数.
 * @retval 无
 * @note 注意nus的取值范围(0~2^32/fac_us)，fac_us一般为系统主频
 * @note 无论是否使用OS，都是用时钟摘取法来做us延时
 */
void delay_us(uint32_t nus)
{
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;        /* LOAD的值 */
    uint32_t ticks = nus * g_fac_us;        /* 要延时的节拍数 */

#if SYS_SUPPORT_OS
    delay_osschedlock();                    /* 锁定 OS 的任务调度器 */
#endif

    told = SysTick->VAL;                    /* 刚进入时的计数器值 */
    while(1){
        tnow = SysTick->VAL;                /* 当前的计数值 */
        if(tnow != told){
            if(tnow < told){
                tcnt += told - tnow;        /* 注意SYSTICK是一个递减的计数器 */
            }else{
                tcnt += reload - tnow + told;   /* 重新装载了一次 */
            }
            told = tnow;
            if(tcnt >= ticks){
                break;                      /* 时间到,则退出 */
            }
        }
    }
    
#if SYS_SUPPORT_OS                          /* 如果需要支持OS */
    delay_osschedunlock();                  /* 恢复 OS 的任务调度器 */
#endif 

}

/**
 * @brief 延时nms微秒
 * @param nms:要延时的微秒数.
 * @retval 无
 * @note 注意nus的取值范围(0~2^32/fac_us/1000)，fac_us一般为系统主频
 */
void delay_ms(uint32_t nms)
{
#if SYS_SUPPORT_OS  /* 如果需要支持OS, 则根据情况调用os延时以释放CPU */
    if(delay_osrunning && delay_osintnesting == 0){     /* OS已经在运行,并且不是在中断里面 */
        if(nms >= g_fac_ms){                            /* 延时的时间大于OS的最少时间周期 */
            delay_ostimedly(nms / g_fac_ms);            /* OS延时 */
        }
        nms %= g_fac_ms;                                /* OS已经无法提供这么小的延时了,采用普通方式延时 */
    }
#endif

    delay_us((uint32_t)(nms * 1000));                   /* 普通方式延时 */

}

/**
 * @brief       HAL库内部函数用到的延时
 * @note        HAL库的延时默认用Systick，如果我们没有开Systick的中断会导致调用这个延时后无法退出
 * @param       Delay : 要延时的毫秒数
 * @retval      None
 */
void HAL_Delay(uint32_t Delay)
{
     delay_ms(Delay);
}
