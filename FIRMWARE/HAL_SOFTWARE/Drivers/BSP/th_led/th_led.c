/**
  ******************************************************************************
  * @file    th_led.c
  * @author  TopHop
  * @brief   
  ******************************************************************************/

#include "th_led/th_led.h"
#include "main.h"


/** 
 * @brief 板间led初始化
 * @param led_mode: LED_MODE_INPUT 输入  LED_MODE_OUTPUT 输出
 * @retval 无 
 */
void th_led_init(uint8_t led_mode)
{
    TH_LED_GPIO_CLK_ENABLE();       // 使能 LED IO 时钟

    GPIO_InitTypeDef th_led_io;
    th_led_io.Pin   = TH_LED_GPIO_PIN;
    th_led_io.Mode  = (led_mode == LED_MODE_INPUT ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT_PP);
    th_led_io.Pull  = GPIO_NOPULL;
    th_led_io.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(TH_LED_GPIO_PORT, &th_led_io);
}

