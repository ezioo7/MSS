/**
 ****************************************************************************************************
 * @file        dataCollect.h
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-12-19
 * @brief       用于处理ADC采集的数据
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 *
 */
#ifndef __MSS_DataCollect_H
#define __MSS_DataCollect_H

#include "stdint.h"

extern float SV_U_MaxI; /* 记录周期内U相的最大电流 */
extern float SV_V_MaxI; /* 记录周期内V相的最大电流 */
extern float SV_W_MaxI; /* 记录周期内W相的最大电流 */

void adc_init(void);
void gpio_data_input_init(void);
void highLevel_dataTrans_init(void);
void highLevel_dataTrans_callback(void);
void lowLevel_dataTrans_task(void *pvParameters);
void data_print_task(void *pvParameters);
void pwm_feedback_it_enable(uint8_t priority);

#endif
