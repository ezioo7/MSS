/**
 ****************************************************************************************************
 * @file        usart.h
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2020-04-20
 * @brief       ���ڳ�ʼ������(һ���Ǵ���1)��֧��printf
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 * �޸�˵��
 * V1.0 20211103
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __USART_H
#define __USART_H

#include "stdio.h"
#include "globalE.h"
#include "./SYSTEM/sys/sys.h"

extern uint8_t g_printf_buffer[CF_PRINTF_BUFFER_SIZE]; /* printf ��������ݻ��Ȼ��浽��ѭ��������, Ȼ������жϷ�ʽ���ͳ�ȥ, ����ʱ�����������ַ� */
extern uint16_t g_printf_head;                         /* ��ͷ*/
extern uint16_t g_printf_end;                          /* ��β, �� head == end ʱ���п�, (end+1)%size == head ʱ����*/

void printf_init(uint32_t baudrate, uint32_t priority); /* ���ڳ�ʼ������ */

#endif
