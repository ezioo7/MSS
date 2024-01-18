/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include "globalE.h"
#include "port.h"
#include "stm32f1xx.h"
#include "./BSP/UART/uart.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
/**
 * @brief   负责开启或关闭读写中断;
 */
void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if (xRxEnable)
    {
        __HAL_UART_ENABLE_IT(&CF_USART_MB_HANDLE, UART_IT_RXNE); // 使能接收非空中断
    }
    else
    {
        __HAL_UART_DISABLE_IT(&CF_USART_MB_HANDLE, UART_IT_RXNE); // 禁能接收非空中断
    }

    if (xTxEnable)
    {

        __HAL_UART_ENABLE_IT(&CF_USART_MB_HANDLE, UART_IT_TXE); // 使能发送为空中断
    }
    else
    {
        __HAL_UART_DISABLE_IT(&CF_USART_MB_HANDLE, UART_IT_TXE); // 禁能发送为空中断
    }
}

/**
 * @brief   初始化modbus使用的串口;
 * @param   ucPort: 用于选择串口, 实际没有用到, 而是采用 CF_USART_MB 来配置; 给 0 即可;
 * @param   ulBaudRate: 串口波特率, 建议不要超过 19200; 因为超过后帧间隔就固定为 1750us;
 * @param   ucDataBits: 数据帧中有效数据的长度, 由于配置了串口固定采用8bit, 所以这一位无效;
 * @param   eParity: 校验方式, 给 MB_PAR_NONE 无校验即可;
 */
BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    uart_enable(CF_USART_MB, ulBaudRate, 1, CF_MB_UART_IT_PRIORITY);
    return TRUE;
}

/**
 * @brief   往串口的DR寄存器写入数据, 相当于写 发送缓冲寄存器;
 */
BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    CF_USART_MB->DR = (uint8_t)ucByte;
    return TRUE;
}

/**
 * @brief   从串口的DR寄存器读数据, 实际是从串口的 读缓冲寄存器 读取数据;
 */
BOOL xMBPortSerialGetByte(CHAR *pucByte)
{
    // DR寄存器只有低8位有效
    *pucByte = (uint8_t)(CF_USART_MB->DR & 0x000000FF);
    return TRUE;
}

/*
 * @brief   回调函数, 应当在触发TXE中断后调用此函数;
 */
void prvvUARTTxReadyISR(void)
{
    /**
     * mb定义好的的发送缓冲区空回调函数, 在eMBInit函数中此函数指针被赋值为xMBRTUTransmitFSM();
     * 在xMBRTUTransmitFSM()中调用了xMBPortSerialPutByte;
     */
    pxMBFrameCBTransmitterEmpty();
    /* 还可以添加自己的处理操作 */
    // do something;
}

/**
 * @brief   回调函数, 应当在触发 RXNE 中断后调用此函数;
 */
void prvvUARTRxISR(void)
{
    /**
     * mb定义好的的接受缓冲区 非空 回调函数, 在eMBInit函数中此函数指针被赋值为 xMBRTUReceiveFSM;
     * 在xMBRTUReceiveFSM()中调用了 xMBPortSerialGetByte;
     */
    pxMBFrameCBByteReceived();
    /* 还可以添加自己的处理操作 */
    // do something;
}
