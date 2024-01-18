/**
 ****************************************************************************************************
 * @file        dataCollect.h
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-19
 * @brief       ���ڴ���ADC�ɼ�������
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 */
#ifndef __MSS_DataCollect_H
#define __MSS_DataCollect_H

#include "stdint.h"

extern float SV_U_MaxI; /* ��¼������U��������� */
extern float SV_V_MaxI; /* ��¼������V��������� */
extern float SV_W_MaxI; /* ��¼������W��������� */

void adc_init(void);
void gpio_data_input_init(void);
void highLevel_dataTrans_init(void);
void highLevel_dataTrans_callback(void);
void lowLevel_dataTrans_task(void *pvParameters);
void data_print_task(void *pvParameters);
void pwm_feedback_it_enable(uint8_t priority);

#endif
