/**
 ****************************************************************************************************
 * @file        gpio.h
 * @author      ������
 * @version     V1.0
 * @date        2023-04-23
 * @brief       GPIO�˿ڼ���Ų���, ����EXTI
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_GPIO_H
#define __MSS_GPIO_H

#include "stm32f1xx.h"

void gpio_clk_enable(void);
void gpio_config(GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t mode, uint32_t pull);
void exti_enable(IRQn_Type IRQn, uint32_t priority);

void led0_config(void);
void led0_on(void);
void led0_off(void);
void led0_toggle(void);

#endif
