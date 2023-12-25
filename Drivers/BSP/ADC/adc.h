/**
 ****************************************************************************************************
 * @file        adc.h
 * @author      ������
 * @version     V1.0
 * @date        2023-04-23
 * @brief       ADC�ɼ�����
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
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
