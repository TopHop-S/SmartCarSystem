/**
  ******************************************************************************
  * @file    usart.c
  * @author  TopHop
  * @brief   串口驱动代码
  *
  *
  *
  ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "./SYSTEM/usart/usart.h"

UART_HandleTypeDef uartx_handle;    /* UART句柄 */
uint8_t aRxbuffer[RXBUFFERSIZE];    /* 接收中断中使用的 */
uint16_t g_usart_rx_sta = 0;        /* 自定义接收状态标记 bit15为1:接收完成 bit14:接收到0x0D bit0~bit13为接收字节数 */
uint8_t g_usart_rx_buf[USART_REC_LEN];  /* 接收缓冲区 */
 
/* Functions  ------------------------------------------------------------------*/

/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)                    /* 使用AC6编译器时 */
__asm(".global __use_no_semihosting\n\t");          /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");            /* AC6下需要声明main函数为无参数格式，否则部分例程可能出现半主机模式 */

#else
/* 使用AC5编译器时, 要在这里定义__FILE 和 不使用半主机模式 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* 不使用半主机模式，至少需要重定义_ttywrch\_sys_exit\_sys_command_string函数,以同时兼容AC6和AC5模式 */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE 在 stdio.h里面定义. */
FILE __stdout;

/* 重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口 */
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0);               /* 等待上一个字符发送完成 */

    USART1->DR = (uint8_t)ch;                       /* 将要发送的字符 ch 写入到DR寄存器 */
    return ch;
}
#endif

/** 
 * @brief 串口 X 初始化函数 
 * @param baudrate: 波特率, 根据自己需要设置波特率值 
 * @note 注意: 必须设置正确的时钟源, 否则串口波特率就会设置异常. 
 * 这里的 USART 的时钟源在 sys_stm32_clock_init()函数中已经设置过了. 
 * @retval 无 
 */
void usart_init(uint32_t baudrate)
{

    uartx_handle.Instance = USART_UX;                       /* USART1 */
    uartx_handle.Init.BaudRate = baudrate;                  /* 波特率 */
    uartx_handle.Init.WordLength = UART_WORDLENGTH_8B;      /* 字长为8位数据格式 */
    uartx_handle.Init.StopBits = UART_STOPBITS_1;           /* 一个停止位 */
    uartx_handle.Init.Parity = UART_PARITY_NONE;            /* 无奇偶校验位 */
    uartx_handle.Init.Mode = UART_MODE_TX_RX;               /* 收发模式 */
    uartx_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;      /* 无硬件流控 */
    HAL_UART_Init(&uartx_handle);

    /* 开启接收中断：标志位 UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量 */
    HAL_UART_Receive_IT(&uartx_handle, (uint8_t *)aRxbuffer, RXBUFFERSIZE);
}

/** 
 * @brief UART 底层初始化函数 
 * @param huart: UART 句柄类型指针 
 * @note 此函数会被 HAL_UART_Init()调用 
 * 完成时钟使能，引脚配置，中断配置  
 * HAL_UART_MspInit 是 HAL 库定义的弱定义函数，这里我们做重定义以实现我们的初始化需求
 * @retval 无 
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init_struct;

    if(huart->Instance == USART_UX){    /* 如果是串口 1，进行串口 1 MSP 初始化 */
        USART_UX_CLK_ENABLE();          /* 使能 USART1 时钟 */
        USART_RX_GPIO_CLK_ENABLE();     /* 使能 USART1 RX GPIO 时钟 */
        USART_TX_GPIO_CLK_ENABLE();     /* 使能 USART1 TX GPIO 时钟 */

        /* USART1 TX GPIO 配置 */
        gpio_init_struct.Pin = USART_TX_GPIO_PIN;       /* PA9 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;        /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_PULLUP;            /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;  /* 高速 */
        gpio_init_struct.Alternate = USART_TX_GPIO_AF;  /* 复用为 USART1 */
        HAL_GPIO_Init(USART_TX_GPIO_PORT,&gpio_init_struct);

        /* USART1 RX GPIO 配置 */
        gpio_init_struct.Pin = USART_RX_GPIO_PIN;       /* PA10 */
        gpio_init_struct.Alternate = USART_RX_GPIO_AF;  /* 复用为 USART1 */
        HAL_GPIO_Init(USART_RX_GPIO_PORT,&gpio_init_struct);

#if USART_EN_RX
        HAL_NVIC_EnableIRQ(USART_UX_IRQn);          /* 使能 USART1 中断通道 */
        HAL_NVIC_SetPriority(USART_UX_IRQn,3,3);    /* 抢占优先级3，子优先级3 */
#endif

    }
}

/*
 * @brief 中断服务函数
 */
void USART_UX_IRQHandler(void)
{
#if SYS_SUPPORT_OS
    OSIntEnter();
#endif

    HAL_UART_IRQHandler(&uartx_handle); /* 调用 HAL 库中断处理公用函数 */

#if SYS_SUPPORT_OS
    OSIntExit();
#endif
}

/** 
 * @brief 重构 UART 数据接收回调接口 数据处理在这里进行 配置为接收1个字节中断一次
 * @param huart:串口句柄 
 * @retval 无 
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART_UX){            // 如果是串口1中断
        if(g_usart_rx_sta & 0x8000 == 0){       // 接收未完成
            if(g_usart_rx_sta & 0x4000){        // 标记已接收到0x0D : 回车键
                if(aRxbuffer[0] != 0x0A){       // 接收到的不是0x0A : 换行键
                    g_usart_rx_sta = 0;         // 清零
                }else{
                    g_usart_rx_sta |= 0x8000;   // 接收到了整个 0x0D 0x0A 完成一次接收
                }
            }else{                              // 还没收到 0x0D（即回车键）: 还未标记接收到
                if(aRxbuffer[0] == 0x0D){       // 若当前字节是 0x0D : 回车键
                    g_usart_rx_sta |= 0x4000;   // 标记就接收到0x0D
                }else{
                    g_usart_rx_buf[g_usart_rx_sta & 0x3FFF] = aRxbuffer[0]; // 正常接收到数据
                    g_usart_rx_sta++;           // 接收字节数增1 
                    if(g_usart_rx_sta > (USART_REC_LEN - 1)){   // 接收数据超出范围
                        g_usart_rx_sta = 0;     // 清零重新开始接收 
                    }
                }
            }
        }
    }
}

