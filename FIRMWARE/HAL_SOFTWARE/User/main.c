/**
 ****************************************************************************************************
 * @file        main.c
 * @author      TopHop
 * @version     V1.0
 * @date        2025-11-01
 * @brief       主函数入口
 * @license     Copyright (c) 2020-2032
 ****************************************************************************************************
 * @attention
 *
 * 系统核心:    STM32F407VET6核心板
 * git地址:     https://github.com/TopHop-S/SmartCarSystem
 * 邮箱地址:    3257136093.com
 *
 ****************************************************************************************************
 */

#include "main.h"
#include "sys/sys.h"
#include "usart/usart.h"
#include "delay/delay.h"
#include "th_tim/th_tim.h"
#include "th_led/th_led.h"
#include "th_key/th_key.h"

// 创建按键管理器
KeyManager_t key_manager;

// 定时器回调（每5ms调用一次）
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    
    if (htim->Instance == TIM7) {  // 使用TIM7作为按键扫描定时器
        // Key_Scan(&key_manager);  // 按键扫描还需优化
    }
}


static void TH_System_Init(void)
{
    sys_stm32_clock_init(336, 8, 2, 7); /* 设置时钟,168Mhz */

    delay_init(168);                    /* 初始化延时函数 */
}

static void TH_BSP_Init(void)
{
    th_debug_uart_init(115200);         /* 初始化串口，波特率115200 */
    printf("串口初始化完成\r\n");

    th_led_init(LED_MODE_INPUT);        /* 初始化LED IO为输出模式 */

    Key_Init(&key_manager);

    /* 时钟频率hz/8400=计数频率，自动重装载为 500，那么 PWM 频率为 计数频率/500=xHz */
    th_tim_pwm1_chy_init(84-1,100-1);     // 168Mhz/84=2MHz, PWM频率=2MHz/100=20KHz
    // th_key_scan_tim_init();             /* 初始化按键扫描定时器 - 按键扫描未完善 */
}

int main(void)
{
    HAL_Init();                         /* 初始化HAL库 */

    TH_System_Init();                   /* 初始化系统 */
    TH_BSP_Init();                      /* 初始化板级设备 */

    printf("当前PWM频率为20KHz,占空比为50\r\n");
    // 默认
    int mpwm_hz = 20000;    // hz单位
    int mpwm_val = 50;      // 占空比0-100

    while(1)
    {
#if 0
        if(th_debug_uart_rx_sta & USART_RX_OVER_BIT)  // 接收完成
        {
            th_debug_uart_rx_buf[th_debug_uart_rx_sta & USART1_RX_DATA_LEN_BIT] = '\0'; // 添加字符串结束符
            printf("接收到的数据：%s\r\n", th_debug_uart_rx_buf);
            th_debug_uart_rx_sta = 0; // 清除状态，准备接收下一个数据包
        }
#endif
       
#if 0
        if(dir)
        {
            ledrpwmval++;
            if(ledrpwmval > 90)
            {
                ledrpwmval = 90;
                dir = 0;
            }
        }
        else
        {
            if(ledrpwmval > 0)
            {
                ledrpwmval--;
            }
            else
            {
                dir = 1;
            }
        }
        set_th_pwm1_duty(ledrpwmval);
#endif 
        delay_ms(10);
        
        if(th_debug_uart_rx_sta & USART_RX_OVER_BIT)  // 接收完成
        {
            th_debug_uart_rx_buf[th_debug_uart_rx_sta & USART_RX_DATA_LEN_BIT] = '\0'; // 添加字符串结束符
            th_debug_uart_rx_sta = 0; // 清除状态，准备接收下一个数据包
            sscanf((const char*)th_debug_uart_rx_buf, "PWM:频率%d 占空比%d", &mpwm_hz,&mpwm_val);
            printf("要设置:%dHz,占空比:%d\r\n", mpwm_hz,mpwm_val);
            switch (mpwm_hz)
            {
                case 5000:
                case 10000:
                case 15000:
                case 20000:
                case 25000:
                case 30000:
                    set_th_pwm1_freq(mpwm_hz);
                    break;
                default:
                    break;
            }

            if(mpwm_val <= 100)
                set_th_pwm1_duty(mpwm_val);

            mpwm_hz     = get_th_pwm1_freq();
            mpwm_val    = get_th_pwm1_duty();
            printf("当前的PWM频率:%dHz,占空比:%d\r\n", mpwm_hz,mpwm_val);
        }
    }
}

/**
 * @brief  全局错误处理
 * @retval None
 */
void Error_Handler(void)
{
    /* 全局禁用中断（把CPU中断屏蔽） */
    __disable_irq();
    while (1)
    {
        /* 错误处理灯闪烁方式 */
    }
}
