/**
 ****************************************************************************************************
 * @file        tim.h
 * @author      电气组
 * @version     V1.0
 * @date        2023-04-23
 * @brief       定时器配置
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 *
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_TIM_H
#define __MSS_TIM_H

#include "stm32f1xx.h"

extern TIM_HandleTypeDef g_tim1_handle;
extern TIM_HandleTypeDef g_tim2_handle;
extern TIM_HandleTypeDef g_tim3_handle;
extern TIM_HandleTypeDef g_tim4_handle;
extern TIM_HandleTypeDef g_tim5_handle;
extern TIM_HandleTypeDef g_tim6_handle;
extern TIM_HandleTypeDef g_tim7_handle;
extern TIM_HandleTypeDef g_tim8_handle;

typedef struct MSS_TIM
{
    TIM_HandleTypeDef *handle;
    TIM_TypeDef *Instance;
    uint8_t APBn;
    uint32_t RCC_en;
    IRQn_Type Update_IRQn;
    

} MSS_TIM;

extern MSS_TIM mss_tim[8];
#define MSS_TIM1 ((MSS_TIM *)mss_tim)
#define MSS_TIM2 (((MSS_TIM *)mss_tim) + 1)
#define MSS_TIM3 (((MSS_TIM *)mss_tim) + 2)
#define MSS_TIM4 (((MSS_TIM *)mss_tim) + 3)
#define MSS_TIM5 (((MSS_TIM *)mss_tim) + 4)
#define MSS_TIM6 (((MSS_TIM *)mss_tim) + 5)
#define MSS_TIM7 (((MSS_TIM *)mss_tim) + 6)
#define MSS_TIM8 (((MSS_TIM *)mss_tim) + 7)

void tim_update_config(MSS_TIM *tim, uint32_t psc, uint32_t arr, uint32_t priority, uint8_t start);
void tim_enable(TIM_HandleTypeDef* htim);
void tim_disable(TIM_HandleTypeDef* htim);
void tim_set_arr(TIM_HandleTypeDef *htim, uint32_t arr);

#endif
