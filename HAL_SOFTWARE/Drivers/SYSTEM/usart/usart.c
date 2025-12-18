/**
  ******************************************************************************
  * @file    usart.c
  * @author  TopHop
  * @brief   参考野火串口通信资料 https://doc.embedfire.com/motor/f407jiaoyang/zh/latest/doc/chapter20/chapter20.html
  *   串口通信驱动，后续需要支持OS
  *
  *
  ******************************************************************************/

#include "sys/sys.h"
#include "usart/usart.h"
#include "main.h"


UART_HandleTypeDef th_debug_uart;
uint8_t th_debug_uart_rx_buf[USART_REC_LEN];    // 接收的数据地址
uint16_t th_debug_uart_rx_sta = 0;              // 接收状态标记
uint8_t g_rx_buf;                       // 用于中断接收单字节数据

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
FILE __stdin;

//重定向c库函数printf到串口USARTx，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
    /* 发送一个字节数据到串口USARTx */
    HAL_UART_Transmit(&th_debug_uart, (uint8_t *)&ch, 1, 0xFFFF);
    /* 等待发送结束 */ 
    while(__HAL_UART_GET_FLAG(&th_debug_uart,UART_FLAG_TC) != SET); 

    return (ch);

#if 0
    while ((TH_DEBUG_UART_BASE->SR & 0X40) == 0);               /* 等待上一个字符发送完成 */

    TH_DEBUG_UART_BASE->DR = (uint8_t)ch;                       /* 将要发送的字符 ch 写入到DR寄存器 */
    return ch;
#endif
}

//重定向c库函数scanf到串口USARTx，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{

    int ch;

    /* 等待串口输入数据 */
    while (__HAL_UART_GET_FLAG(&th_debug_uart, UART_FLAG_RXNE) == RESET);
    HAL_UART_Receive(&th_debug_uart, (uint8_t *)&ch, 1, 0xFFFF);

    return (ch);
}

#endif


/**
 * @brief  串口1初始化
 * @param  bound: 波特率
 * @retval None
 */
void th_debug_uart_init(uint32_t bound)
{
    th_debug_uart.Instance          = TH_DEBUG_UART_BASE;   // 串口寄存器地址
    th_debug_uart.Init.BaudRate     = bound;				// 波特率
    th_debug_uart.Init.WordLength 	= UART_WORDLENGTH_8B;	// 字长
    th_debug_uart.Init.StopBits     = UART_STOPBITS_1;      // 停止位
    th_debug_uart.Init.Parity       = UART_PARITY_NONE;     // 校验位
    th_debug_uart.Init.Mode         = UART_MODE_TX_RX;      // 收发模式
    th_debug_uart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;  // 流控制
    th_debug_uart.Init.OverSampling = UART_OVERSAMPLING_16; // 16倍过采样-数据精确

    if(HAL_UART_Init(&th_debug_uart) != HAL_OK)
    {
        Error_Handler();
    }

    /*开启接收中断：标志位 UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量*/
    HAL_UART_Receive_IT(&th_debug_uart,(uint8_t *)&g_rx_buf,1);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init_str = {0};
    if(huart->Instance == TH_DEBUG_UART_BASE){  // debug串口初始化
        TH_DEBUG_UART_CLK_ENABLE();         // 使能USART时钟
        TH_DEBUG_GPIO_TXCLK_ENABLE();       // 使能GPIO时钟-发送
        TH_DEBUG_GPIO_RXCLK_ENABLE();       // 使能GPIO时钟-接收
        /* 发送引脚初始化 */
        gpio_init_str.Pin   = TH_DEBUG_UART_TX_PIN;
        gpio_init_str.Mode  = GPIO_MODE_AF_PP;
        gpio_init_str.Pull  = GPIO_PULLUP;
        gpio_init_str.Speed = GPIO_SPEED_FREQ_HIGH;
        gpio_init_str.Alternate = TH_DEBUG_UART_AF;
        HAL_GPIO_Init(TH_DEBUG_GPIO_TX_TYPE,&gpio_init_str);

        /* 接收引脚初始化 */
        gpio_init_str.Pin   = TH_DEBUG_UART_RX_PIN;
        HAL_GPIO_Init(TH_DEBUG_GPIO_RX_TYPE,&gpio_init_str);

        /* 配置并使能USART1中断 */
        HAL_NVIC_EnableIRQ(TH_DEBUG_IRQn_TYPE);
        HAL_NVIC_SetPriority(TH_DEBUG_IRQn_TYPE, 3, 3);

    }
}

// 接收中断服务函数
void TH_DEBUG_UART_IRQHandler(void)
{
#if SYS_SUPPORT_OS /* 使用 OS */
    OSIntEnter();
#endif

    /* 调用 HAL 库中断处理公用函数，其中会调用 HAL_UART_RxCpltCallback */
    HAL_UART_IRQHandler(&th_debug_uart);

#if SYS_SUPPORT_OS /* 使用 OS */ 
    OSIntExit();
#endif
}

/**
 * @brief       Rx传输完成回调函数
 * @param       huart: UART句柄类型指针
 * @retval      无
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == TH_DEBUG_UART_BASE)
    {
        if((th_debug_uart_rx_sta & USART_RX_OVER_BIT) == 0)    // 接收未完成
        {
            // windows下通常回车换行是0x0D 0x0A
            if(th_debug_uart_rx_sta & USART_RX_0X0D_BIT)       // 接收到了0x0D
            {
                if(g_rx_buf != 0x0A){
                    th_debug_uart_rx_sta = 0;                   // 不是0x0A，接收错误，重新开始接收
                }else{
                    th_debug_uart_rx_sta |= USART_RX_OVER_BIT; // 接收完成
                }
            }else{  // 之前还没收到0x0D
                if(g_rx_buf == 0x0D){    // 如果数据是0x0D
                    th_debug_uart_rx_sta |= USART_RX_0X0D_BIT;     // 标记接收到0x0D
                }else{
                    // 正常接收数据
                    th_debug_uart_rx_buf[th_debug_uart_rx_sta & USART_RX_DATA_LEN_BIT] = g_rx_buf;
                    th_debug_uart_rx_sta ++;            // 接收到的数据长度加1
                    if((th_debug_uart_rx_sta & USART_RX_DATA_LEN_BIT) > (USART_REC_LEN-1))
                    {   // 接收数据溢出
                        th_debug_uart_rx_sta = 0;       // 重新开始接收
                    }
                }
            }
        }
        // 重新启动接收中断，准备接收下一个字节
        HAL_UART_Receive_IT(&th_debug_uart,(uint8_t *)&g_rx_buf,1);
    }
}


