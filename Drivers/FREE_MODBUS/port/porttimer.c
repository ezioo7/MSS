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
 * @brief   配置定时器, 该定时器每隔50us计数一次
 * @param   usTim1Timerout50us: 溢出计数, 需要直接赋给ARR, 不要 - 1;
 */
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    /**
     * 定时器需要每隔50us计数一次, 即时钟频率 20KHz, 计算得分频系数3600;
     * 由于tim_update_config内部自动对传入的 arr 进行 -1 , 所以这里传入 usTim1Timerout50us + 1 
     */
    tim_update_config(CF_TIM_MB, 3600, usTim1Timerout50us + 1, 10, FALSE);
    return TRUE;
}

/**
 * @brief   使能定时器;
*/
inline void
vMBPortTimersEnable()
{

    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
    tim_enable(&CF_TIM_MB_HANDLE);
}

/**
 * @brief   失能定时器;
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
     * 当定时器发生更新中断时, 说明自收到上个字符后已经过去 t35, 调用回调函数;
    */
    (void)pxMBPortCBTimerExpired();
}
