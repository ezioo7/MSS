/**
 ****************************************************************************************************
 * @file        usart.c
 * @author      电器组,wangdexu
 * @version     V1.0
 * @date        2023-12-11
 * @brief       串口初始化代码，支持printf
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 *
 * 修改说明
 * V1.0 20211103
 * 第一次发布
 *
 ****************************************************************************************************
 */
#include "./globalA.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/UART/uart.h"

/******************************************************************************************/
/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if 1

#if (__ARMCC_VERSION >= 6010050)           /* 使用AC6编译器时 */
__asm(".global __use_no_semihosting\n\t"); /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");   /* AC6下需要声明main函数为无参数格式，否则部分例程可能出现半主机模式 */

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

/******************************************************************************/
/*                             需要关注的部分                                  */
/******************************************************************************/
uint8_t g_printf_buffer[CF_PRINTF_BUFFER_SIZE] = {0}; /* printf 输出的内容会先缓存到此循环队列中, 然后采用中断方式发送出去, 队满时丢弃新来的字符 */
uint16_t g_printf_head = 0;                            /* 队头, 指向队内第一个元素 */
uint16_t g_printf_end = 0;                             /* 队尾, 指向下一个元素的插入位置, 当 head == end 时队列空, (end+1)%size == head 时队满*/
/**
 * @brief 判断缓冲数组是否还有空间, 0 = 队列已满, 1 = 仍有空间;
 */
inline static uint8_t buffer_available()
{
    return !(((g_printf_end + 1) % CF_PRINTF_BUFFER_SIZE) == g_printf_head);
}

/* MDK下需要重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口 */
/* TODO: 修改为中断实现, 现在的实现方式在实时环境下很难用 */
int fputc(int ch, FILE *f)
{
    /**
     * 如果队列不满
     *      将ch写到缓存数组, 修改end, 使能TXR
     * 如果满了, 无动作, 相当于丢弃新来的字符
     */
    if (buffer_available())
    {
        g_printf_buffer[g_printf_end] = ch;
        g_printf_end = (g_printf_end + 1) % CF_PRINTF_BUFFER_SIZE;
        __HAL_UART_ENABLE_IT(&CF_USART_PRINTF_HANDLE, UART_IT_TXE);
    }
    return ch;
}
#endif

/**
 * @brief       串口X初始化函数
 * @param       baudrate: 波特率, 根据自己需要设置波特率值
 * @param       priority: 串口中断的优先级
 * @note        注意: 必须设置正确的时钟源, 否则串口波特率就会设置异常.
 *              这里的USART的时钟源在sys_stm32_clock_init()函数中已经设置过了.
 * @retval      无
 */
void printf_init(uint32_t baudrate, uint32_t priority)
{
    uart_enable(CF_USART_PRINTF, baudrate, 1, priority);
}
