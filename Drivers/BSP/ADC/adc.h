/**
 ****************************************************************************************************
 * @file        adc.h
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-11-23
 * @brief       ADC采集程序
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 * 
 * 
 */
#ifndef __MSS_ADC_H
#define __MSS_ADC_H

#include "stm32f1xx.h"

/* global variables */
extern ADC_HandleTypeDef g_adc1_handle;
extern DMA_HandleTypeDef g_dma1_handle;

void adc_dma_enable(const uint32_t *channels);

#endif
