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
 * @brief   ��������رն�д�ж�;
 */
void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if (xRxEnable)
    {
        __HAL_UART_ENABLE_IT(&CF_USART_MB_HANDLE, UART_IT_RXNE); // ʹ�ܽ��շǿ��ж�
    }
    else
    {
        __HAL_UART_DISABLE_IT(&CF_USART_MB_HANDLE, UART_IT_RXNE); // ���ܽ��շǿ��ж�
    }

    if (xTxEnable)
    {

        __HAL_UART_ENABLE_IT(&CF_USART_MB_HANDLE, UART_IT_TXE); // ʹ�ܷ���Ϊ���ж�
    }
    else
    {
        __HAL_UART_DISABLE_IT(&CF_USART_MB_HANDLE, UART_IT_TXE); // ���ܷ���Ϊ���ж�
    }
}

/**
 * @brief   ��ʼ��modbusʹ�õĴ���;
 * @param   ucPort: ����ѡ�񴮿�, ʵ��û���õ�, ���ǲ��� CF_USART_MB ������; �� 0 ����;
 * @param   ulBaudRate: ���ڲ�����, ���鲻Ҫ���� 19200; ��Ϊ������֡����͹̶�Ϊ 1750us;
 * @param   ucDataBits: ����֡����Ч���ݵĳ���, ���������˴��ڹ̶�����8bit, ������һλ��Ч;
 * @param   eParity: У�鷽ʽ, �� MB_PAR_NONE ��У�鼴��;
 */
BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    uart_enable(CF_USART_MB, ulBaudRate, 1, CF_MB_UART_IT_PRIORITY);
    return TRUE;
}

/**
 * @brief   �����ڵ�DR�Ĵ���д������, �൱��д ���ͻ���Ĵ���;
 */
BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    CF_USART_MB->DR = (uint8_t)ucByte;
    return TRUE;
}

/**
 * @brief   �Ӵ��ڵ�DR�Ĵ���������, ʵ���ǴӴ��ڵ� ������Ĵ��� ��ȡ����;
 */
BOOL xMBPortSerialGetByte(CHAR *pucByte)
{
    // DR�Ĵ���ֻ�е�8λ��Ч
    *pucByte = (uint8_t)(CF_USART_MB->DR & 0x000000FF);
    return TRUE;
}

/*
 * @brief   �ص�����, Ӧ���ڴ���TXE�жϺ���ô˺���;
 */
void prvvUARTTxReadyISR(void)
{
    /**
     * mb����õĵķ��ͻ������ջص�����, ��eMBInit�����д˺���ָ�뱻��ֵΪxMBRTUTransmitFSM();
     * ��xMBRTUTransmitFSM()�е�����xMBPortSerialPutByte;
     */
    pxMBFrameCBTransmitterEmpty();
    /* ����������Լ��Ĵ������ */
    // do something;
}

/**
 * @brief   �ص�����, Ӧ���ڴ��� RXNE �жϺ���ô˺���;
 */
void prvvUARTRxISR(void)
{
    /**
     * mb����õĵĽ��ܻ����� �ǿ� �ص�����, ��eMBInit�����д˺���ָ�뱻��ֵΪ xMBRTUReceiveFSM;
     * ��xMBRTUReceiveFSM()�е����� xMBPortSerialGetByte;
     */
    pxMBFrameCBByteReceived();
    /* ����������Լ��Ĵ������ */
    // do something;
}
