/**
  ******************************************************************************
  * @file    th_key.c
  * @author  TopHop
  * @brief   
  ******************************************************************************/

#include "th_key/th_key.h"
#include "delay/delay.h"
#include "main.h"

static uint8_t sw2_index;      // 板间按键SW2索引

/**
 * @brief 按键消抖采样函数
 * @param key 按键结构体指针
 * @return 0:稳定无效电平(释放), 1:稳定有效电平(按下), 2:抖动状态
 */
static uint8_t Key_ReadPhysicalState(Key_t *key)
{
    if (key == NULL){
        return 0;
    }

    uint8_t samples = 0;

    // 采样5次
    for (int i = 0; i < 5; i++) {
        if (HAL_GPIO_ReadPin(key->port, key->pin) == key->active_level) {
            samples |= (1 << i);
        }
        delay_us(200);      // 微秒级延时
    }

    // 统计低电平次数（按下次数）
    uint8_t count_low = 0;
    for (int i = 0; i < 5; i++) {
        if (samples & (1 << i)) count_low++;
    }
    
    // 判断逻辑（低电平有效）：
    // 5次中有4次以上低电平 → 确认按下
    // 5次中有1次以下低电平 → 确认释放
    // 其他 → 抖动
    if (count_low >= 4) {
        return 1;      // 稳定按下（低电平）
    } else if (count_low <= 1) {
        return 0;      // 稳定释放（高电平）
    } else {
        return 2;      // 抖动状态
    }
}

/** 
 * @brief 板间KEY-SW2初始化
 * @retval 无 
 */
static void th_key_sw2_init(void)
{
    TH_KEY_SW2_GPIO_CLK_ENABLE();       // 使能 key IO 时钟

    GPIO_InitTypeDef th_key_io;
    th_key_io.Pin   = TH_KEY_SW2_GPIO_PIN;
    th_key_io.Mode  = GPIO_MODE_INPUT;
    th_key_io.Pull  = GPIO_PULLUP;
    th_key_io.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(TH_KEY_SW2_GPIO_PORT, &th_key_io);
}

// 按键初始化
void Key_Init(KeyManager_t *manager)
{
    if(manager == NULL)
        return;
    
    manager->scan_interval = 5;     // 默认扫描间隔5ms
    manager->key_num = 0;
    for(uint8_t i = 0; i < TH_KEY_MAX_NUM; i++){
        manager->keys[i].state = KEY_STATE_IDLE;
        manager->keys[i].event = KEY_EVENT_NONE;
        manager->keys[i].index = 0;
    }

    th_key_sw2_init();
    sw2_index = Key_Add(manager, TH_KEY_SW2_GPIO_PORT, TH_KEY_SW2_GPIO_PIN, TH_KEY_SW2_GPIO_ACTIVE);    // 添加板间按键SW2 - 索引0
     
}

// 按键添加
uint8_t Key_Add(KeyManager_t *manager,GPIO_TypeDef *port, uint16_t pin, uint8_t active_level)
{
    if (manager == NULL || (manager->key_num >= TH_KEY_MAX_NUM))
    {
        return (uint8_t)-1;
    }

    uint8_t index = manager->key_num;
    manager->keys[index].port   = port;
    manager->keys[index].pin    = pin;
    manager->keys[index].active_level = active_level;
    manager->keys[index].state  = KEY_STATE_IDLE;
    manager->keys[index].event  = KEY_EVENT_NONE;
    manager->keys[index].click_count = 0;
    manager->keys[index].last_click_time = 0;
    manager->keys[index].index  = index;
    manager->key_num++;

    return index;
}

// 按键扫描状态机
void Key_Scan(KeyManager_t *manager)
{
    if(manager == NULL) return;

    for(uint8_t i=0;i<manager->key_num;i++){
        Key_t *key = &manager->keys[i];
        uint8_t current_state = Key_ReadPhysicalState(key);

        // 状态机处理
        switch(key->state){
            case KEY_STATE_IDLE:            // 空闲
                if (current_state == 1){
                    key->state = KEY_STATE_PRESS_DOWN;
                    key->tick_count = 0;
                    printf("已按键,待消抖\r\n");
                }
                
                break;
            case KEY_STATE_PRESS_DOWN:      // 按下消抖
                if (current_state == 1){
                    key->tick_count += manager->scan_interval;
                    if(key->tick_count >= TH_KEY_DEBOUNCE_TICKS){   // 达到消抖时间，确认按下
                        key->state = KEY_STATE_PRESS;
                        key->event = KEY_EVENT_SHORT_DOWN;
                        key->tick_count = 0;
                    }
                }else{
                    key->state = KEY_STATE_IDLE;    // 抖动，返回空闲
                }
                
                break;
            case KEY_STATE_PRESS:           // 按下稳定
                if (current_state == 1){
                    key->tick_count += manager->scan_interval;
                    if(key->tick_count >= TH_KEY_LONGPRESS_TICKS){  // 达到长按时间，进入长按状态
                        key->state = KEY_STATE_LONG_PRESS;
                        key->event = KEY_EVENT_LONG_DOWN;
                        key->tick_count = 0;
                    }
                }else{
                    key->state = KEY_STATE_RELEASE_UP;  // 释放消抖
                    key->tick_count = 0;
                }
                break;
            case KEY_STATE_RELEASE_UP:      // 释放消抖
                if(current_state != 1){
                    key->tick_count += manager->scan_interval;
                    if(key->tick_count >= TH_KEY_DEBOUNCE_TICKS){   // 达到消抖时间，确认释放
                        key->state = KEY_STATE_IDLE;
                        key->event = KEY_EVENT_SHORT_UP;

                        // 检测单击或双击
                        uint32_t current_time = HAL_GetTick();
                        if (current_time - key->last_click_time < 500) {
                            key->click_count++;
                            if (key->click_count >= 2) {
                                key->event = KEY_EVENT_DOUBLE_CLICK;
                                key->click_count = 0;
                            }
                        } else {
                            key->click_count = 1;
                            key->event = KEY_EVENT_CLICK;
                        }
                        key->last_click_time = current_time;
                    }
                }else{
                    key->state = KEY_STATE_PRESS;   // 抖动，返回按下状态
                }
                break;
            case KEY_STATE_LONG_PRESS:      // 长按
                if(current_state == 1){
                    key->tick_count += manager->scan_interval;
                    if(key->tick_count >= TH_KEY_REPEAT_TICKS){     // 达到连续按键时间，进入重复状态
                        // key->state = KEY_STATE_REPEAT;
                        key->event = KEY_EVENT_REPEAT;
                        key->tick_count = 0;

                    }
                }else{
                    key->state = KEY_STATE_RELEASE_UP;
                    key->event = KEY_EVENT_LONG_UP;
                    key->tick_count = 0;
                }
                break;
            // case KEY_STATE_REPEAT:          // 重复
            //     break;

            default:
                key->state = KEY_STATE_IDLE;
                break;
        }
    }
}

KeyEvent_t Key_GetEvent(KeyManager_t *manager, uint8_t key_index)
{
    if (manager == NULL || key_index >= manager->key_num) {
        return KEY_EVENT_NONE;
    }

    KeyEvent_t event = manager->keys[key_index].event;
    // manager->keys[key_index].event = KEY_EVENT_NONE;    // 清除事件
    return event;
}

uint8_t Get_SW2_Index(void)
{
    return sw2_index;
}

