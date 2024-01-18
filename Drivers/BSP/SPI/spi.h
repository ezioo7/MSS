/**
 ****************************************************************************************************
 * @file        spi.h
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-01-06
 * @brief       SPI驱动
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_SPI_H
#define __MSS_SPI_H

#include "stm32f1xx.h"

extern SPI_HandleTypeDef g_spi1_handle;
extern SPI_HandleTypeDef g_spi3_handle;

void spi1_init(uint32_t baud_Rate_Prescaler);

#endif
