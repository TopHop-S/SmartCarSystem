/**
  ******************************************************************************
  * @file    th_tim.c
  * @author  TopHop
  * @brief   
  *     定时器配置文件，提供定时器初始化及相关操作接口
  ******************************************************************************/

#include "th_tim/th_tim.h"
#include "main.h"

TIM_HandleTypeDef th_pwm1_tim;
TIM_HandleTypeDef th_key_scan_tim;

/** 
 * @brief 高级定时器 TIMX 通道 Y 输出指定个数 PWM 初始化函数 
 * @note 
 * 高级定时器的时钟来自 APB2, 而 PCLK2 = 168Mhz, 我们设置 PPRE2 不分频, 因此 
 * 高级定时器时钟 = 168Mhz 
 * 定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us. 
 * Ft=定时器工作频率,单位:Mhz 
 * @param psc: 时钟预分频数 
 * @param arr: 自动重装值 
 * @retval 无 
 */
void th_tim_pwm1_chy_init(uint32_t psc, uint32_t arr)
{
    th_pwm1_tim.Instance = TH_PWM1_TIM_BASE;                          // 定时器基地址
    th_pwm1_tim.Init.Prescaler           = psc;                             // 预分频值
    th_pwm1_tim.Init.Period              = arr;                             // 自动重装载值
    th_pwm1_tim.Init.CounterMode         = TIM_COUNTERMODE_UP;              // 计数模式
    th_pwm1_tim.Init.ClockDivision       = TIM_CLOCKDIVISION_DIV1;          // 时钟分频
    th_pwm1_tim.Init.RepetitionCounter   = 0;                               // 重复计数器值
    th_pwm1_tim.Init.AutoReloadPreload   = TIM_AUTORELOAD_PRELOAD_ENABLE;   // 自动重装载寄存器ARR的预装载使能

    if(HAL_OK != HAL_TIM_PWM_Init(&th_pwm1_tim))
    {
        Error_Handler();
    }

    /* 设置比较值,此值用来确定占空比，默认比较值为自动重装载值的一半,即占空比为 50% */
    // 默认占空比50%
    TIM_OC_InitTypeDef oc_pwm_chy = {0};
    oc_pwm_chy.OCMode       = TIM_OCMODE_PWM1;              // PWM模式1
    oc_pwm_chy.Pulse        = (arr+1)>>1;                   // 占空比
    oc_pwm_chy.OCPolarity   = TIM_OCPOLARITY_HIGH;          // 输出极性高
    HAL_TIM_PWM_ConfigChannel(&th_pwm1_tim,&oc_pwm_chy,TH_PWM1_TIM_CHY);

    // !!! 对于高级定时器的PWM正常输出，必须使能MOE位，可使用这一行来使能主输出 !!! 或 使用以下的 AutomaticOutput
    th_pwm1_tim.Instance->BDTR |= TIM_BDTR_MOE;  // 使能主输出

#if 0
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;        // 运行模式下的输出空闲状态
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;       // 空闲模式下的输出空闲状态
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;             // 锁定级别
    sBreakDeadTimeConfig.DeadTime = 0;                              // 死区时间
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;            // 断路器功能关闭
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_LOW;     // 断路器极性高
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;  // 自动输出使能
    HAL_TIMEx_ConfigBreakDeadTime(&th_pwm1_tim, &sBreakDeadTimeConfig);  // 配置死区时间和断路器
#endif
#if 0
    HAL_NVIC_SetPriority(TH_PWM1_TIM_IRQn,1,3);
    HAL_NVIC_EnableIRQ(TH_PWM1_TIM_IRQn);

    __HAL_TIM_ENABLE_IT(&th_pwm1_tim,TIM_IT_UPDATE);    // 运行更新中断
#endif
    HAL_TIM_PWM_Start(&th_pwm1_tim,TH_PWM1_TIM_CHY);        // 开启PWM通道
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    
    if(htim->Instance == TH_PWM1_TIM_BASE)
    {
        /* 使能定时器时钟 */
        TH_PWM1_TIM_CLK_ENABLE();
        TH_PWM1_TIM_CHANNEL_CLK_ENABLE();
        
        /* 配置PWM通道GPIO */
        GPIO_InitTypeDef pwm1_io;
        pwm1_io.Pin     = TH_PWM1_GPIO_PIN;
        pwm1_io.Mode    = GPIO_MODE_AF_PP;
        pwm1_io.Pull    = GPIO_PULLUP;
        pwm1_io.Speed   = GPIO_SPEED_FREQ_HIGH;
        pwm1_io.Alternate = TH_PWM1_GPIO_AF;
        HAL_GPIO_Init(TH_PWM1_GPIO_PORT, &pwm1_io);
    }
}

// 返回 0 成功，-1失败
int set_th_pwm1_freq(uint32_t freq_hz)
{
    if(freq_hz == 0)    return -1;

    // 取得当前时钟频率
    uint32_t pclk = TH_PWM1_TIM_CLKFREQ();
    uint32_t old_psc = th_pwm1_tim.Init.Prescaler;
    uint32_t old_arr = th_pwm1_tim.Init.Period;
    uint32_t old_ccr = __HAL_TIM_GET_COMPARE(&th_pwm1_tim, TH_PWM1_TIM_CHY);

    // 以当前psc计算ARR，这里直接使用初始化的psc了，所以一开始的psc要设置正确
    uint64_t counter_freq = (uint64_t)pclk / (old_psc + 1); // 计数器频率
    if(counter_freq == 0)   return -1;
    // 计算新频率后的ARR值
    uint32_t new_arr = counter_freq / freq_hz;
    if(new_arr == 0)  return -1;
    new_arr -= 1;

    // 计算新的 CCR 以保持相同占空比比例
    uint32_t new_ccr = 0;
    if (old_arr > 0) {
        new_ccr = (uint32_t)((uint64_t)old_ccr * (new_arr + 1) / (old_arr + 1));
    }

    // 安全更新：停止 -> 写 PSC/ARR/CCR -> 触发更新 -> 启动
    HAL_TIM_PWM_Stop(&th_pwm1_tim, TH_PWM1_TIM_CHY);
    HAL_Delay(100);

    // 更新(直接写寄存器)
    th_pwm1_tim.Instance->PSC   = old_psc;
    th_pwm1_tim.Init.Prescaler  = old_psc;
    th_pwm1_tim.Instance->ARR   = new_arr;
    th_pwm1_tim.Init.Period     = new_arr;
    __HAL_TIM_SET_COMPARE(&th_pwm1_tim, TH_PWM1_TIM_CHY,new_ccr);

    // 软件触发更新
    th_pwm1_tim.Instance->EGR = TIM_EGR_UG;

    // 对TIM1/TIM8确保MOE
    if(th_pwm1_tim.Instance == TIM1 || th_pwm1_tim.Instance == TIM8){
        th_pwm1_tim.Instance->BDTR |= TIM_BDTR_MOE;
    }

    HAL_TIM_PWM_Start(&th_pwm1_tim, TH_PWM1_TIM_CHY);

    return 0;
}

void set_th_pwm1_duty(uint32_t duty)
{
    if(duty > 100) duty = 100;   // 最大占空比100%
    uint32_t pwm_duty = (th_pwm1_tim.Init.Period + 1) * duty / 100;
    __HAL_TIM_SET_COMPARE(&th_pwm1_tim, TH_PWM1_TIM_CHY, pwm_duty);
}

uint32_t get_th_pwm1_freq(void)
{
    /* 计算当前 PWM 频率： timer_clk / ((PSC+1) * (ARR+1)) */
    uint32_t timer_clk = TH_PWM1_TIM_CLKFREQ();
    uint32_t psc = th_pwm1_tim.Instance->PSC;
    uint32_t arr = th_pwm1_tim.Instance->ARR;
// printf("clk:%d  psc:%d  arr:%d\r\n",timer_clk,psc,arr);
    if ((psc == 0xFFFFFFFF) || (arr == 0xFFFFFFFF) || (psc == 0)) {
        return 0;
    }

    uint64_t denom = (uint64_t)(psc + 1) * (uint64_t)(arr + 1);
    if (denom == 0) return 0;

    uint32_t freq = (uint32_t)((uint64_t)timer_clk / denom);
    return freq;
}

uint32_t get_th_pwm1_duty(void)
{
    /* 返回当前占空比，单位：百分比（0-100） */
    uint32_t arr = th_pwm1_tim.Instance->ARR;
    uint32_t ccr = __HAL_TIM_GET_COMPARE(&th_pwm1_tim, TH_PWM1_TIM_CHY);
// printf("arr:%d  ccr:%d\r\n",arr,ccr);
    if (arr == 0) return 0;

    uint32_t duty = (uint32_t)(((uint64_t)ccr * 100ULL) / (arr + 1));
    if (duty > 100) duty = 100;
    return duty;
}


void th_key_scan_tim_init(void)
{
    // 配置5MS定时器作为按键扫描定时器
    
    th_key_scan_tim.Instance = TIM7;
    th_key_scan_tim.Init.Prescaler = 84 - 1;        // 84Mhz/84=1MHz
    th_key_scan_tim.Init.Period = 5000 - 1;         // 1MHz/5000=200Hz, 5ms中断一次
    th_key_scan_tim.Init.CounterMode = TIM_COUNTERMODE_UP;

    HAL_TIM_Base_Init(&th_key_scan_tim);

    HAL_TIM_Base_Start_IT(&th_key_scan_tim); /* 使能定时器 x 和定时器 x 更新中断 */ 

}


void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) 
{ 
    if (htim->Instance == TIM7) 
    { 
        __HAL_RCC_TIM7_CLK_ENABLE(); /* 使能 TIMx 时钟 */ 
        /* 设置中断优先级，抢占优先级 2，子优先级 3 */ 
        HAL_NVIC_SetPriority(TIM7_IRQn, 2, 3); 
        HAL_NVIC_EnableIRQ(TIM7_IRQn); /* 开启 ITMx 中断 */
        printf("TIM7已初始化 \r\n");
    } 
}

/**
  * @brief  TIM7 中断服务程序
  * @retval None
  */
void TIM7_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&th_key_scan_tim);
}



