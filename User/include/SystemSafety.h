/**
 ****************************************************************************************************
 * @file        SystemSafety.h
 * @author      电气组，zhuowenwei
 * @version     V1.0
 * @date        2024-12-09
 * @brief       系统错误
 * @licence     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 * 
 * 
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_SystemSafety_H
#define __MSS_SystemSafety_H

#include "globalE.h"

extern uint8_t  SV_Sequence_Detect_Flag;           /*标志是否需要进行相序检测*/
extern uint32_t SV_UPWM_Count;                     /*单个周期内 U相上下桥导通计数，每个周期的初始为0*/
extern uint32_t SV_VPWM_Count;                     /*单个周期内 V相上下桥导通计数，每个周期的初始为0*/
extern uint32_t SV_WPWM_Count;                     /*单个周期内 W相上下桥导通计数，每个周期的初始为0*/

extern float SV_OVERPHI_L1Cnt;                     /*1.05倍额定电流，运行时间，单位，精度*/
extern float SV_OVERPHI_L2Cnt;                     /*1.2倍额定电流，运行时间，单位，精度*/
extern float SV_OVERPHI_L3Cnt;                     /*1.5倍额定电流，运行时间，单位，精度*/
extern float SV_OVERPHI_L4Cnt;                     /*1.8倍额定电流，运行时间，单位，精度*/
extern float SV_OVERPHI_L5Cnt;                     /*2倍额定电流，运行时间，单位，精度*/
extern float SV_OVERPHI_L6Cnt;                     /*3倍额定电流，运行时间，单位，精度*/

uint16_t CheckPhaseSequence(void);          /*相序错误监测*/


    
void OverLoadCurrent(void);                 /*过载反时限保护，默认为4级反时限保护，2倍电流为过流*/
    /*系统输出1.05倍额定电流过载限时保护,6分钟*/
    /*系统输出1.2倍额定电流过载限时保护，2分钟*/
    /*系统输出1.5倍额定电流过载限时保护，1分钟*/
    /*系统输出1.8倍额定电流过载限时保护,30秒*/
    /*系统输出2倍额定电流过载限时保护，8秒*/
    /*系统输出3倍额定电流过载限时保护，2秒*/
uint16_t OverCurrent_Factor(float OverPhITime, float LimitTime, float OverFactor);


#endif
