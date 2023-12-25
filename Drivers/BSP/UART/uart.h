/**
 ****************************************************************************************************
 * @file        uart.h
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-14
 * @brief       ��������
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 * 
 * 
 ****************************************************************************************************
 */
#ifndef __MSS_UART_H
#define __MSS_UART_H

#include "stm32f1xx.h"

extern UART_HandleTypeDef g_uart1_handle;
extern UART_HandleTypeDef g_uart2_handle;
extern UART_HandleTypeDef g_uart3_handle;
extern UART_HandleTypeDef g_uart4_handle;
extern UART_HandleTypeDef g_uart5_handle;

void uart_enable(USART_TypeDef *Instance, uint32_t baudrate, uint8_t itEnable, uint32_t priority);

#endif
