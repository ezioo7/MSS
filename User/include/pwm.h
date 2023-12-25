/**
 ****************************************************************************************************
 * @file        pwm.h
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-19
 * @brief       ����PWM����
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 */
#include "stm32f1xx.h"

void pwm_config(void);
void pwm_enable(void);
void pwm_disable(void);
void pwm_update_callback(TIM_HandleTypeDef *tim_handle);
