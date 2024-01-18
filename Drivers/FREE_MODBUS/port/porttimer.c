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

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
#include "globalE.h"
#include "stm32f1xx.h"
#include "./BSP/TIM/tim.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
/**
 * @brief   ���ö�ʱ��, �ö�ʱ��ÿ��50us����һ��
 * @param   usTim1Timerout50us: �������, ��Ҫֱ�Ӹ���ARR, ��Ҫ - 1;
 */
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    /**
     * ��ʱ����Ҫÿ��50us����һ��, ��ʱ��Ƶ�� 20KHz, ����÷�Ƶϵ��3600;
     * ����tim_update_config�ڲ��Զ��Դ���� arr ���� -1 , �������ﴫ�� usTim1Timerout50us + 1 
     */
    tim_update_config(CF_TIM_MB, 3600, usTim1Timerout50us + 1, 10, FALSE);
    return TRUE;
}

/**
 * @brief   ʹ�ܶ�ʱ��;
*/
inline void
vMBPortTimersEnable()
{

    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
    tim_enable(&CF_TIM_MB_HANDLE);
}

/**
 * @brief   ʧ�ܶ�ʱ��;
*/
inline void
vMBPortTimersDisable()
{
    /* Disable any pending timers. */
    tim_disable(&CF_TIM_MB_HANDLE);
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
void prvvTIMERExpiredISR(void)
{
    /**
     * ����ʱ�����������ж�ʱ, ˵�����յ��ϸ��ַ����Ѿ���ȥ t35, ���ûص�����;
    */
    (void)pxMBPortCBTimerExpired();
}
