/**
  ******************************************************************************
  * @file    sys.c
  * @author  TopHop
  * @brief   
  *
  *   This file provides two functions and one global variable to be called from 
  *   user application:
  *
  *
  ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "./SYSTEM/sys/sys.h"

/* Functions  ------------------------------------------------------------------*/

/**
 * @brief 时钟配置函数
 * @param plln 主PLL倍频系数，取值范围64~432
 * @param pllm 主PLL和音频PLL分频系数（进PLL之前的分频），取值范围2~63
 * @param pllp 主PLL的p分频系数（PLL之后的分频），分频后作为系统时钟，取值范围2、4、6、8
 * @param pllq 主PLL的q分频系数（PLL之后的分频），分频后作为USB、SDIO、RNG时钟，取值范围2~15
 * @note
 *      1. 系统时钟频率计算公式：SYSCLK = (HSE_VALUE / pllm) * plln / pllp
 *      2. USB、SDIO、RNG时钟频率计算公式：USB_CLK = (HSE_VALUE / pllm) * plln / pllq
 *  Fvco: VCO频率
 *  Fsys: 系统时钟频率, 也是主 PLL的 p分频输出时钟频率
 *  Fq: 主 PLL的 q分频输出时钟频率
 *  Fs: 主 PLL输入时钟频率, 可以是 HSI, HSE等
 *  Fvco = Fs * (plln / pllm);
 *  Fsys = Fvco / pllp = Fs * (plln / (pllm * pllp));
 *  Fq   = Fvco / pllq = Fs * (plln / (pllm * pllq));
 *      外部晶振为8Mhz，CPU主频为168Mhz，推荐参数配置为plln=336, pllm=8, pllp=2, pllq=7
 *      Fvco = 8Mhz * (336 / 8) = 336Mhz
 *      Fsys = 336Mhz / 2 = 168Mhz = pll_p_ck
 *      Fq   = 336Mhz / 7 = 48Mhz
 * 
 *      F407默认需要配置的频率如下:
 *      CPU频率(HCLK) = pll_p_ck = 168Mhz
 *      AHB1/2/3(rcc_hclk1/2/3) = 168Mhz
 *      APB1(rcc_pclk1) = pll_p_ck / 4 = 42Mhz
 *      APB1(rcc_pclk2) = pll_p_ck / 2 = 84Mhz
 * 
 * @retval 0:成功; 1:失败
 */
uint8_t sys_stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    HAL_StatusTypeDef ret = HAL_OK;
    RCC_OscInitTypeDef rcc_osc_init = {0};
    RCC_ClkInitTypeDef rcc_clk_init = {0};

    /** 使能PWR时钟 */
    __HAL_RCC_PWR_CLK_ENABLE();

    /** 设置内部线性调压器输出电压级别，以保证在高主频（比如 STM32F407 的 168 MHz）下能稳定工作 */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** 使能HSE，并选择HSE作为PLL时钟源，配置PLL1，开启USB时钟 */
    rcc_osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    rcc_osc_init.HSEState = RCC_HSE_ON;                 /* 使能HSE */
    rcc_osc_init.PLL.PLLState = RCC_PLL_ON;             /* 打开PLL */
    rcc_osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;     /* 选择HSE作为PLL时钟源 */
    rcc_osc_init.PLL.PLLN = plln;
    rcc_osc_init.PLL.PLLM = pllm;
    switch(pllp){
        case 2: rcc_osc_init.PLL.PLLP = RCC_PLLP_DIV2; break;
        case 4: rcc_osc_init.PLL.PLLP = RCC_PLLP_DIV4; break;
        case 6: rcc_osc_init.PLL.PLLP = RCC_PLLP_DIV6; break;
        case 8: rcc_osc_init.PLL.PLLP = RCC_PLLP_DIV8; break;
        default:
            /* invalid pllp value */
            return 1;
    }
    rcc_osc_init.PLL.PLLQ = pllq;

    ret = HAL_RCC_OscConfig(&rcc_osc_init);
    if(ret != HAL_OK){
        return 1;
    }

    /** 选中PLL作为系统时钟源并且配置HCLK,PCLK1和PCLK2 */
    rcc_clk_init.ClockType = (RCC_CLOCKTYPE_SYSCLK \
                            | RCC_CLOCKTYPE_HCLK \
                            | RCC_CLOCKTYPE_PCLK1 \
                            | RCC_CLOCKTYPE_PCLK2 \
                            );
    
    /** 设置系统时钟时钟源为PLL */
    rcc_clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    rcc_clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;           /* AHB分频系数为1 */
    rcc_clk_init.APB1CLKDivider = RCC_HCLK_DIV4;            /* APB1分频系数为4 */
    rcc_clk_init.APB2CLKDivider = RCC_HCLK_DIV2;            /* APB2分频系数为2 */
    
    /** 同时设置FLASH延时周期为5WS，也就是6个CPU周期 */
    ret = HAL_RCC_ClockConfig(&rcc_clk_init,FLASH_LATENCY_5);
    if(ret != HAL_OK){
        return 1;
    }

    /** STM32F405x/407x/415x/417x 版本的器件支持预取功能 */
    #ifdef STM32F407xx
        __HAL_FLASH_PREFETCH_BUFFER_ENABLE();   /* 使能flash预取指令 */
    #endif

    return 0;
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief       当编译提示出错的时候此函数用来报告错误的文件和所在行
 * @param       file：指向源文件
 *              line：指向在文件中的行数
 * @retval      无
 */
void assert_failed(uint8_t* file, uint32_t line)
{ 
    while (1)
    {
    }
}
#endif
