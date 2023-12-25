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
#include "stdint.h"

void adc_init(void);
void gpio_data_input_init(void);
void highLevel_dataTrans_init(void);
void highLevel_dataTrans_callback(void);
void lowLevel_dataTrans_task(void *pvParameters);
void data_print_task(void *pvParameters);
void pwm_feedback_it_enable(uint8_t priority);

