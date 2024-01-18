/**
 ****************************************************************************************************
 * @file        SystemSafety.h
 * @author      �����飬zhuowenwei
 * @version     V1.0
 * @date        2024-12-09
 * @brief       ϵͳ����
 * @licence     Copyright (c) 2020-2032, �����г����������޹�˾
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

extern uint8_t  SV_Sequence_Detect_Flag;           /*��־�Ƿ���Ҫ����������*/
extern uint32_t SV_UPWM_Count;                     /*���������� U�������ŵ�ͨ������ÿ�����ڵĳ�ʼΪ0*/
extern uint32_t SV_VPWM_Count;                     /*���������� V�������ŵ�ͨ������ÿ�����ڵĳ�ʼΪ0*/
extern uint32_t SV_WPWM_Count;                     /*���������� W�������ŵ�ͨ������ÿ�����ڵĳ�ʼΪ0*/

extern float SV_OVERPHI_L1Cnt;                     /*1.05�������������ʱ�䣬��λ������*/
extern float SV_OVERPHI_L2Cnt;                     /*1.2�������������ʱ�䣬��λ������*/
extern float SV_OVERPHI_L3Cnt;                     /*1.5�������������ʱ�䣬��λ������*/
extern float SV_OVERPHI_L4Cnt;                     /*1.8�������������ʱ�䣬��λ������*/
extern float SV_OVERPHI_L5Cnt;                     /*2�������������ʱ�䣬��λ������*/
extern float SV_OVERPHI_L6Cnt;                     /*3�������������ʱ�䣬��λ������*/

uint16_t CheckPhaseSequence(void);          /*���������*/


    
void OverLoadCurrent(void);                 /*���ط�ʱ�ޱ�����Ĭ��Ϊ4����ʱ�ޱ�����2������Ϊ����*/
    /*ϵͳ���1.05�������������ʱ����,6����*/
    /*ϵͳ���1.2�������������ʱ������2����*/
    /*ϵͳ���1.5�������������ʱ������1����*/
    /*ϵͳ���1.8�������������ʱ����,30��*/
    /*ϵͳ���2�������������ʱ������8��*/
    /*ϵͳ���3�������������ʱ������2��*/
uint16_t OverCurrent_Factor(float OverPhITime, float LimitTime, float OverFactor);


#endif
