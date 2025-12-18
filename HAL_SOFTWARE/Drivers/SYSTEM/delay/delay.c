/**
  ******************************************************************************
  * @file    delay.c
  * @author  TopHop
  * @brief   延时函数驱动文件
  *
  ******************************************************************************/

#include "sys/sys.h"
#include "delay/delay.h"


static uint32_t g_fac_us = 0;       /* us 延时倍乘数（根据系统时钟设置，单位：MHz） */

/* 如果启用了操作系统支持 (例如 uC/OS-II)，下面提供与 OS 协同工作的实现 */
#if SYS_SUPPORT_OS

/* uC/OS-II 头文件（当使用 OS 时需要） */
#include "os.h"

/* g_fac_ms: OS 下 millisecond 计数的缩放因子，表示多少 ms 对应一个 OS tick */
static uint16_t g_fac_ms = 0;

/*
 * 为了支持在有操作系统 (OS) 的情况下延时，代码中使用了几项 OS 相关的变量/宏：
 *  - delay_osrunning:   表示 OS 是否已启动并运行（通常映射到 OSRunning）
 *  - delay_ostickspersec: 表示 OS 每秒的时钟节拍数（OS_TICKS_PER_SEC）
 *  - delay_osintnesting: 表示当前中断嵌套层级（OSIntNesting），用于判断是否在中断上下文中
 *
 *  并提供三个与 OS 交互的函数：
 *  - delay_osschedlock/ delay_osschedunlock: 在需要精确微秒级延时期间锁住/解锁调度，防止任务切换
 *  - delay_ostimedly: 使用 OS 的延时函数按 tick 延时
 */

/* 映射到 uC/OS-II 的符号 */
#define delay_osrunning     OSRunning
#define delay_ostickspersec OS_TICKS_PER_SEC
#define delay_osintnesting  OSIntNesting


/**
 * @brief  在需要保证短时间内不发生任务切换时调用（例如精确微秒级延时）
 *         该函数会锁住任务调度（uC/OS-II 的 OSSchedLock），从而防止任务切换。
 */
void delay_osschedlock(void)
{
    OSSchedLock();
}


/**
 * @brief  解除任务调度锁（对应 OSSchedUnlock），恢复任务切换
 */
void delay_osschedunlock(void)
{
    OSSchedUnlock();
}


/**
 * @brief  使用操作系统的延时接口按 tick 延时
 * @param  ticks: 要延时的 OS tick 数
 */
void delay_ostimedly(uint32_t ticks)
{
    OSTimeDly(ticks);
}


/**
 * @brief  SysTick 中断处理函数，当使用 OS 时需将 SysTick 交给 OS 处理
 *         同时仍调用 HAL_IncTick() 以维护 HAL 的毫秒计数。
 */
void SysTick_Handler(void)
{
    /* 如果 OS 已启动，则将 SysTick 事件传递给 OS 内核 */
    if (delay_osrunning == OS_TRUE)
    {
        OS_CPU_SysTickHandler();
    }
    /* 更新 HAL 的系统滴答计数 */
    HAL_IncTick();
}
#endif

/**
 * @brief  初始化延时函数
 * @param  sysclk: 系统时钟频率，单位 MHz（例如 sysclk=168 表示 168MHz）
 * @retval None
 */
void delay_init(uint16_t sysclk)
{
#if SYS_SUPPORT_OS                                      /* 如果支持 OS，则做额外的 SysTick 配置 */
    uint32_t reload;
#endif
    /* g_fac_us 用于计算微秒延时所需的计数倍数（与系统时钟相关） */
    g_fac_us = sysclk;
#if SYS_SUPPORT_OS
    /* 在 OS 下，需要将 SysTick 配置为 OS 的时钟节拍源 */
    reload = sysclk;                                    /* 以 MHz 为单位 */
    reload *= 1000000 / delay_ostickspersec;            /* 计算每次 OS tick 的 SysTick 计数值 */
    /* g_fac_ms 表示多少毫秒等于一个 OS tick，用于 delay_ms 中的换算 */
    g_fac_ms = 1000 / delay_ostickspersec;
    SysTick->CTRL |= 1 << 1;                            /* 使能 SysTick 中断 */
    SysTick->LOAD = reload;                             /* 设置 SysTick 重装载值 */
    SysTick->CTRL |= 1 << 0;                            /* 启动 SysTick */
#endif
}

/**
 * @brief  精确延时（微秒级）
 * @note   该函数基于 SysTick 计数实现精确延时；当启用 OS 时，会短时锁住调度以保证精度
 * @param  nus: 要延时的微秒数（us）
 * @note   nus 取值范围大致为 0 ~ (2^32 / g_fac_us)，具体取决于系统时钟
 * @retval None
 */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;        /* SysTick 重装载寄存器的当前值 */
    ticks = nus * g_fac_us;                 /* 需要等待的计数（目标计时单位：SysTick ticks） */
    
#if SYS_SUPPORT_OS
    delay_osschedlock();
#endif

    told = SysTick->VAL; /* 记录当前计数寄存器的值 */
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break; /* 到达所需延时，退出循环 */
            }
        }
    }

#if SYS_SUPPORT_OS
    delay_osschedunlock(); /* 释放任务调度锁 */
#endif

}

/**
 * @brief  毫秒级延时
 * @param  nms: 要延时的毫秒数（ms）
 * @retval None
 * @note   如果支持 OS 且 OS 已运行且不在中断上下文中，会尽量使用 OS 的延时接口以节省 CPU
 */
void delay_ms(uint16_t nms)
{
#if SYS_SUPPORT_OS
    /* 如果 OS 已经启动并且当前不在中断上下文，则优先使用 OS 的延时（按 tick） */
    if (delay_osrunning && delay_osintnesting == 0)
    {
        if (nms >= g_fac_ms)
        {
            /* 当延时大于等于一个 OS tick 时，使用 OS 延时以让出 CPU */
            delay_ostimedly(nms / g_fac_ms);
        }

        /* 处理剩余的小于一个 tick 的毫秒数，使用忙等待 */
        nms %= g_fac_ms;
    }
#endif

    /* 以微秒延时函数实现剩余毫秒延时 */
    delay_us((uint32_t)(nms * 1000));
}

/**
 * @brief  HAL 层内部使用的延时函数封装（以毫秒为单位）
 * @note   HAL 库期望提供一个 HAL_Delay 接口，这里将其转发到 delay_ms
 * @param  Delay: 要延时的毫秒数
 * @retval None
 */
void HAL_Delay(uint32_t Delay)
{
    delay_ms(Delay);
}


