/**
 ****************************************************************************************************
 * @file        usart.h
 * @author      电器组,wangdexu
 * @version     V1.0
 * @date        2020-04-20
 * @brief       串口初始化代码(一般是串口1)，支持printf
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

#ifndef __USART_H
#define __USART_H

#include "stdio.h"
#include "globalE.h"
#include "./SYSTEM/sys/sys.h"

extern uint8_t g_printf_buffer[CF_PRINTF_BUFFER_SIZE]; /* printf 输出的内容会先缓存到此循环队列中, 然后采用中断方式发送出去, 队满时丢弃新来的字符 */
extern uint16_t g_printf_head;                         /* 队头*/
extern uint16_t g_printf_end;                          /* 队尾, 当 head == end 时队列空, (end+1)%size == head 时队满*/

void printf_init(uint32_t baudrate, uint32_t priority); /* 串口初始化函数 */

#endif
