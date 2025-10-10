/**
  ******************************************************************************
  * @file    key.h
  * @author  TopHop
  * @date    2025/10/10
  * @version V1.0
  * @brief   Header file for key.c module KEY 驱动代码
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __KEY_H__
#define __KEY_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "./SYSTEM/sys/sys.h"

/* 物理按键KEY0引脚定义 */
#define KEY0_GPIO_PORT  GPIOA
#define KEY0_GPIO_PIN   GPIO_PIN_0

/* 物理按键KEY0 PA口时钟使能 */
#define KEY0_GPIO_CLK_ENABLE()  do{ __HAL_RCC_GPIOA_CLK_ENABLE();}while (0);

/* 读取物理KEY0引脚 */
#define KEY0_STATUS     HAL_GPIO_ReadPin(KEY0_GPIO_PORT,KEY0_GPIO_PIN)

/* 物理KEY按下状态 */
enum{
    KEY_NONE,
    KEY0_PRES = 1,      /* KEY0 按下 */
};

/* 外部接口函数 */
void key_init(void);    /* 初始化 */
uint8_t key_scan(void); /* 按键扫描函数, 有响应优先级, 同时按下多个按键时, 只能响应优先级最高的那个 */

#ifdef __cplusplus
}
#endif


#endif /* __KEY_H__ */
