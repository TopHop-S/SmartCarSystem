/**
  ******************************************************************************
  * @file           : th_key.h
  * @brief          : Header for th_key.c file.
  *
  ******************************************************************************
  * @attention  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TH_KEY_H__
#define __TH_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private defines -----------------------------------------------------------*/

#define TH_KEY_SW2_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()    // LED IO 时钟使能

// GPIO配置
#define TH_KEY_SW2_GPIO_PORT            GPIOA                           // LED IO TypeDef
#define TH_KEY_SW2_GPIO_PIN             GPIO_PIN_0                      // LED IO PIN
#define TH_KEY_SW2_GPIO_ACTIVE          (0)                             // 有效电平

// 按键配置
#define TH_KEY_MAX_NUM                  1                               // 最大按键数量
#define TH_KEY_DEBOUNCE_TICKS           20                              // 按键消抖延时，单位毫秒
#define TH_KEY_LONGPRESS_TICKS          1000                            // 长按时间，单位毫秒
#define TH_KEY_REPEAT_TICKS             300                             // 连续按键时间，单位毫秒

// 按键事件
typedef enum{
    KEY_EVENT_NONE = 0,     // 无事件
    KEY_EVENT_SHORT_DOWN,   // 短按按下
    KEY_EVENT_SHORT_UP,     // 短按释放
    KEY_EVENT_LONG_DOWN,    // 长按按下
    KEY_EVENT_LONG_UP,      // 长按释放
    KEY_EVENT_REPEAT,       // 长按重复
    KEY_EVENT_CLICK,        // 单击
    KEY_EVENT_DOUBLE_CLICK, // 双击
    KEY_EVENT_HOLD          // 持续按住
}KeyEvent_t;

// 按键状态机
typedef enum{
    KEY_STATE_IDLE = 0,     // 空闲
    KEY_STATE_PRESS_DOWN,   // 按下消抖
    KEY_STATE_PRESS,        // 按下稳定
    KEY_STATE_RELEASE_UP,   // 释放消抖
    KEY_STATE_LONG_PRESS,   // 长按
    KEY_STATE_REPEAT        // 重复
}KeyState_t;

// 按键结构体
typedef struct{
    GPIO_TypeDef    *port;          // 按键GPIO端口
    uint16_t        pin;            // 按键GPIO引脚
    uint8_t         active_level;   // 按键有效电平(0或1)

    uint8_t         index;          // 按键索引（在管理器的位置）
    KeyState_t      state;          // 当前状态
    uint32_t        tick_count;     // 计时器计数
    KeyEvent_t      event;          // 当前事件
    // 高级功能
    uint8_t         click_count;    // 点击计数（用于双击）
    uint32_t        last_click_time;// 上次点击时间
}Key_t;

// 按键管理器
typedef struct{
    Key_t           keys[TH_KEY_MAX_NUM];   // 按键数组
    uint8_t         key_num;                // 按键数量
    uint8_t         scan_interval;          // 扫描间隔(ms)
}KeyManager_t;


/* Exported functions prototypes ---------------------------------------------*/

void Key_Init(KeyManager_t *manager);
uint8_t Key_Add(KeyManager_t *manager,GPIO_TypeDef *port, uint16_t pin, uint8_t active_level);
void Key_Scan(KeyManager_t *manager);
KeyEvent_t Key_GetEvent(KeyManager_t *manager, uint8_t key_index);
uint8_t Get_SW2_Index(void);

#ifdef __cplusplus
}
#endif

#endif /* __TH_KEY_H__ */
