/**
 ****************************************************************************************************
 * @file        pwm.h
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-12-19
 * @brief       用于PWM发波
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 *
 */
#ifndef __MSS_PWMSoft_H
#define __MSS_PWMSoft_H

#include "stm32f1xx.h"

#ifdef CF_PWM_USE_SOFT
void pwm_config(void);
void pwm_enable(void);
void pwm_disable(void);
void pwm_update_IT_callback(TIM_HandleTypeDef *tim_handle);
#endif

#endif
