/**
 ****************************************************************************************************
 * @file        pwm3TIM.h
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-01-10
 * @brief       pwm����ģ��,3��ʱ��ʵ�ְ汾
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 * 
 *
 ****************************************************************************************************
 */
#ifndef __MSS_PWM3TIM_H
#define __MSS_PWM3TIM_H

#include "stm32f1xx.h"

#ifdef CF_PWM_USE_3TIM
void pwm_config(void);
void pwm_enable(void);
void pwm_disable(void);

void pwm_phaseA_IT_callback(void);
void pwm_pullup_IT_callback(void);
void pwm_pulldown_IT_callback(void);
#endif

#endif
